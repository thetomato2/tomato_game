namespace tom
{

struct GameState;

struct SimEntityHash
{
    SimEntity* ptr;
    u32 ind;
};

struct sim_region
{
    WorldPos origin;
    r3f bounds;
    r3f update_bounds;

    u32 max_sim_entity_cnt;
    u32 sim_entity_cnt;
    SimEntity* sim_entities;

    // todo: change size? need hash?
    // note: must be a power of 2
    SimEntityHash hash[4096];
};

}  // namespace tom