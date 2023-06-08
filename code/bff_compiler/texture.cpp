#include "../bff_file/g_bff.h"

void GetTexture(const json& data, bffinfo_t* info)
{
    info->numTextures = data["textures"].size();
    if (info->numTextures >= MAX_TEXTURE_CHUNKS) {
        BFF_Error("WriteBFF: the number of textures in entries file exceeds the limit of %i textures", MAX_TEXTURE_CHUNKS);
    }
    memset(info->textures, 0, sizeof(info->textures));
    for (bff_int_t i = 0; i < info->numTextures; i++) {
        bfftexture_t* texture = &info->textures[i];
        const std::string texturenode = "texture_"+std::to_string(i);
        if (!data["textures"].contains(texturenode)) {
            BFF_Error("WriteBFF: texture node %s wasn't found, expected more textures", texturenode.c_str());
        }
        if (!data["textures"][texturenode].contains("texfile")) {
            BFF_Error("WriteBFF: texture node %i doesn't contain a texfile", i);
        }
        const std::string texturefile = data["textures"][texturenode]["texfile"];
        FILE* fp = SafeOpen(texturefile.c_str(), "rb");
        LoadFile(fp, (void **)&texture->fileBuffer, &texture->fileSize);
        fclose(fp);
    }
}