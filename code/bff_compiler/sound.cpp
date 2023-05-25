#include "../bff_file/g_bff.h"

void GetSounds(const json& data, bffinfo_t* info)
{
    info->numSounds = data["sounds"].size();
    if (info->numSounds >= MAX_SOUND_CHUNKS) {
        BFF_Error("WriteBFF: the number of sounds in entries file exceeds the limit of %i sounds", MAX_SOUND_CHUNKS);
    }
    memset(info->sounds, 0, sizeof(info->sounds));
    for (bff_int_t i = 0; i < info->numSounds; i++) {
        bffsound_t* sound = &info->sounds[i];

        const std::string soundnode = "sound_"+std::to_string(i);
        if (!data["sounds"].contains(soundnode)) {
            BFF_Error("WriteBFF: could not find sound node %s", soundnode.c_str());
        }
        if (data["sounds"][soundnode].contains("name")) {
			const std::string name = data["sounds"][soundnode]["name"];
			Con_Printf("loading sound %s", name.c_str());
			if (name.size() > MAX_BFF_CHUNKNAME - 1) {
				Con_Printf("WARNING: sound name at index %lu is greater than %i characters, truncating", i, MAX_BFF_CHUNKNAME - 1);
			}
			strncpy(sound->name, name.c_str(), MAX_BFF_CHUNKNAME - 1);
		}
		else {
			Con_Printf("WARNING: sound at index %lu doesn't have an assigned name, using default name of %s", soundnode.c_str());
			strncpy(sound->name, soundnode.c_str(), MAX_BFF_CHUNKNAME - 1);
		}
        if (!data["sounds"][soundnode].contains("soundfile")) {
            BFF_Error("WriteBFF: each sound node must contain a file path, sound node %i does not", i);
        }
        const std::string soundfile = data["sounds"][soundnode]["soundfile"];
        if (strcasestr(soundfile.c_str(), ".ogg") == NULL
        && strcasestr(soundfile.c_str(), ".wav") == NULL
        && strcasestr(soundfile.c_str(), ".opus") == NULL) {
            BFF_Error("WriteBFF: sound file type at index %i isn't supported yet, use ogg, wav, or opus", i);
        }
        if (strcasestr(soundfile.c_str(), ".ogg") != NULL) {
            sound->fileType = SFT_OGG;
        }
        else if (strcasestr(soundfile.c_str(), ".wav") != NULL) {
            sound->fileType = SFT_WAV;
        }
        else if (strcasestr(soundfile.c_str(), ".opus") != NULL) {
            sound->fileType = SFT_OPUS;
        }
        FILE* fp = SafeOpen(soundfile.c_str(), "rb");
        LoadFile(fp, (void **)&sound->fileBuffer, &sound->fileSize);
        fclose(fp);
    }
}