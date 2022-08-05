namespace tom
{
struct Texture
{
    enum class Type
    {

        none,
        R8,
        R8G8B8A8,
    };

    i32 width, height;
    Type type;
    void* buf;
};

fn i32 texture_get_num_channels(Texture::Type type)
{
    switch (type) {
        case Texture::Type::none: return 0;
        case Texture::Type::R8: return 1;
        case Texture::Type::R8G8B8A8: return 4;
    }

    return -1;
};

fn Texture::Type texture_type_from_channels(i32 n_channels)
{
    switch (n_channels) {
        case 0: return Texture::Type::none;
        case 1: return Texture::Type::R8;
        case 4: return Texture::Type::R8G8B8A8;
    }

    return Texture::Type::none;
};

// FIXME:this leaks but I don't care atm
// TODO: use a memroy arena? 
fn Texture texture_load_from_file(const char* path)
{
    Texture result;
    i32 n_channels;

    // stbi_set_flip_vertically_on_load(true);
    result.buf  = (void*)stbi_load(path, &result.width, &result.height, &n_channels, 0);
    result.type = texture_type_from_channels(n_channels);

    return result;
}

}