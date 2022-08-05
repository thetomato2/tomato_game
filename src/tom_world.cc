
namespace tom
{

function bool is_canonical(f32 rel_coord)
{
    return rel_coord >= chunk_size_meters * -0.5f - eps_f32 &&
           rel_coord <= chunk_size_meters * 0.5f + eps_f32;
}

function bool is_canonical(v3f rel_coord)
{
    return is_canonical(rel_coord.x) && is_canonical(rel_coord.y) && is_canonical(rel_coord.z);
}

fn void recanonicalize_coord(i32& coord, f32& rel_coord)
{
    // NOTE: World is assumed to be toroidal (torus shaped World),
    // if you step off one end where you wrap around
    i32 offset = round_f32_to_i32(rel_coord / (f32)chunk_size_meters);
    coord += offset;
    rel_coord -= offset * (f32)chunk_size_meters;

    Assert(is_canonical(rel_coord));
}

fn bool is_same_chunk(WorldPos a, WorldPos b)
{
    Assert(is_canonical(a.offset));
    Assert(is_canonical(b.offset));

    return (a.chunk_x == b.chunk_x && a.chunk_y == b.chunk_y && a.chunk_z == b.chunk_z);
}

fn WorldPos get_centered_point(i32 x, i32 y, i32 z)
{
    WorldPos result;

    result.chunk_x = x;
    result.chunk_y = y;
    result.chunk_z = z;

    return result;
}

fn void init_world(World* World, f32 tile_sizes_in_meters)
{
    World->first_free = nullptr;
    for (i32 chunk_i = 0; chunk_i < CountOf(World->world_chunk_hash); ++chunk_i) {
        World->world_chunk_hash[chunk_i].x                   = CHUNK_UNITIALIZED;  // null chunk
        World->world_chunk_hash[chunk_i].first_block.ent_cnt = 0;
    }
}

fn v3f get_world_diff(WorldPos pos_a, WorldPos pos_b)
{
    v3f dif;
    dif.x = (f32)pos_a.chunk_x - (f32)pos_b.chunk_x;
    dif.y = dif.y = (f32)pos_a.chunk_y - (f32)pos_b.chunk_y;
    dif.z = dif.z = (f32)pos_a.chunk_z - (f32)pos_b.chunk_z;

    v3f result = vec_hadamard(chunk_dim_meters, dif) + (pos_a.offset - pos_b.offset);

    return result;
}

fn WorldPos map_into_chunk_space(WorldPos pos, v3f offset)
{
    auto result = pos;

    // TODO: decide on tile chunk alignment
    result.offset += offset;
    recanonicalize_coord(result.chunk_x, result.offset.x);
    recanonicalize_coord(result.chunk_y, result.offset.y);
    recanonicalize_coord(result.chunk_z, result.offset.z);

    return result;
}

fn WorldPos map_into_chunk_space(WorldPos pos, v2f offset)
{
    v3f offset_v3 = v3_init(offset);
    auto result   = map_into_chunk_space(pos, offset_v3);

    return result;
}

fn WorldChunk* get_world_chunk(World* World, i32 chunk_x, i32 chunk_y, i32 chunk_z,
                                     Arena* arena = nullptr)
{
    Assert(chunk_x > -chunk_safe_margin);
    Assert(chunk_y > -chunk_safe_margin);
    Assert(chunk_z > -chunk_safe_margin);
    Assert(chunk_x < chunk_safe_margin);
    Assert(chunk_y < chunk_safe_margin);
    Assert(chunk_z < chunk_safe_margin);

    // TODO: better hash function!
    i32 hash_val  = 19 * chunk_x + 7 * chunk_y + 3 * chunk_z;
    i32 hash_slot = (i32)hash_val & (CountOf(World->world_chunk_hash) - 1);
    Assert(hash_slot < CountOf(World->world_chunk_hash));

    WorldChunk* chunk = World->world_chunk_hash + hash_slot;
    do {
        // found chunk
        if (chunk_x == chunk->x && chunk_y == chunk->y && chunk_z == chunk->z) {
            break;
        }
        // didn't find chunk but there isn't a next chunk
        // so allocate a new one and move the pointer there
        if (arena && chunk->x == CHUNK_UNITIALIZED && !chunk->next_in_hash) {
            chunk->next_in_hash = push_struct<WorldChunk>(arena);
            chunk               = chunk->next_in_hash;
            chunk->x            = CHUNK_UNITIALIZED;
        }

        // if chunk is empty (0) allocate the tiles
        if (arena && chunk->x == CHUNK_UNITIALIZED) {
            chunk->x = chunk_x;
            chunk->y = chunk_y;
            chunk->z = chunk_z;

            // do we want to always initialize?
            chunk->next_in_hash = nullptr;
            break;
        }
        chunk = chunk->next_in_hash;
    } while (chunk);

    return chunk;
}

fn WorldPos abs_pos_to_world_pos(f32 abs_x, f32 abs_y, f32 abs_z)
{
    WorldPos result;

    result.chunk_x = (i32)(abs_x / chunk_dim_meters.x);
    result.chunk_y = (i32)(abs_y / chunk_dim_meters.y);
    result.chunk_z = (i32)(abs_z / chunk_dim_meters.z);

    result.offset.x = abs_x - (result.chunk_x * chunk_dim_meters.x);
    result.offset.y = abs_y - (result.chunk_y * chunk_dim_meters.y);
    result.offset.z = abs_z - (result.chunk_z * chunk_dim_meters.z);

    // TODO: use map_into_chunk_space?
    recanonicalize_coord(result.chunk_x, result.offset.x);
    recanonicalize_coord(result.chunk_y, result.offset.y);
    recanonicalize_coord(result.chunk_z, result.offset.z);

    return result;
}

fn void change_entity_location_raw(Arena* arena, World* World, u32 ent_i, WorldPos* old_pos,
                                         WorldPos* new_pos)
{
    if (is_valid(new_pos)) {
        if (is_valid(old_pos) && is_same_chunk(*old_pos, *new_pos)) {
            //  leave the Entity where it is
            return;
        } else {
            if (is_valid(old_pos)) {
                // pull thee*ntity out its old block
                WorldChunk* chunk =
                    get_world_chunk(World, old_pos->chunk_x, old_pos->chunk_y, old_pos->chunk_z);
                Assert(chunk);
                if (chunk) {
                    bool found                    = false;
                    WorldEntityBlock* first_block = &chunk->first_block;
                    for (WorldEntityBlock* block = &chunk->first_block; block && !found;
                         block                   = block->next) {
                        for (u32 i {}; i < block->ent_cnt; ++i) {
                            if (block->ent_inds[i] == ent_i) {
                                Assert(first_block->ent_cnt > 0);
                                block->ent_inds[i] = first_block->ent_inds[--first_block->ent_cnt];
                                if (first_block->ent_cnt == 0) {
                                    if (first_block->next) {
                                        WorldEntityBlock* next_block = first_block->next;
                                        *first_block                 = *next_block;
                                        next_block->next             = World->first_free;
                                        World->first_free            = next_block;
                                    }
                                }
                                found = true;
                            }
                        }
                    }
                }
            }

            //  insert h*e Entity into its new block
            WorldChunk* chunk =
                get_world_chunk(World, new_pos->chunk_x, new_pos->chunk_y, new_pos->chunk_z, arena);
            Assert(chunk);
            WorldEntityBlock* block = &chunk->first_block;
            if (block->ent_cnt == CountOf(block->ent_inds)) {
                // out of room! make new block
                // REVIEW: this sem*s kinda backwards to me, it's like a backwards pointing list
                WorldEntityBlock* old_block = World->first_free;
                if (old_block) {
                    World->first_free = old_block->next;
                } else {
                    old_block = push_struct<WorldEntityBlock>(arena);
                }
                *old_block     = *block;
                block->next    = old_block;
                block->ent_cnt = 0;
            }

            Assert(block->ent_cnt < CountOf(block->ent_inds));
            block->ent_inds[block->ent_cnt++] = ent_i;
        }
    }
}

fn void change_entity_location(Arena* arena, World* World, Entity* ent,
                                     WorldPos new_pos_init)
{
    WorldPos *old_pos = nullptr, *new_pos = nullptr;
    if (!is_flag_set(ent->sim.flags, sim_entity_flags::nonspatial)) old_pos = &ent->world_pos;
    if (is_valid(new_pos_init)) new_pos = &new_pos_init;

    change_entity_location_raw(arena, World, ent->sim.ent_i, old_pos, new_pos);
    if (new_pos) {
        ent->world_pos = *new_pos;
        clear_flags(ent->sim.flags, sim_entity_flags::nonspatial);
    } else {
        ent->world_pos = null_world_pos();
        set_flags(ent->sim.flags, sim_entity_flags::nonspatial);
    }
}

}  // namespace tom