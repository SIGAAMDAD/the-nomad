#include "../bff_file/g_bff.h"

void GetTextures(const json& data, bffinfo_t* info)
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
        if (data["textures"][texturenode].contains("name")) {
			const std::string name = data["textures"][texturenode]["name"];
			Con_Printf("loading texture %s", name.c_str());
			if (name.size() > MAX_BFF_CHUNKNAME - 1) {
				Con_Printf("WARNING: texture name at index %lu is greater than %i characters, truncating", i, MAX_BFF_CHUNKNAME - 1);
			}
			strncpy(texture->name, name.c_str(), MAX_BFF_CHUNKNAME - 1);
		}
		else {
			Con_Printf("WARNING: texture at index %lu doesn't have an assigned name, using default name of %s", i, texturenode.c_str());
			strncpy(texture->name, texturenode.c_str(), MAX_BFF_CHUNKNAME - 1);
		}
        if (!data["textures"][texturenode].contains("texfile")) {
            BFF_Error("WriteBFF: texture node %i doesn't contain a texfile", i);
        }
        const std::string texturefile = data["textures"][texturenode]["texfile"];
        if (!strcasestr(texturefile.c_str(), ".jpg") && !strcasestr(texturefile.c_str(), ".jpeg")) {
            BFF_Error("WriteBFF: texture file must be a jpeg");
        }
        FILE* fp = SafeOpen(texturefile.c_str(), "rb");
        LoadFile(fp, (void **)&texture->fileBuffer, &texture->fileSize);
        fclose(fp);
    }
}