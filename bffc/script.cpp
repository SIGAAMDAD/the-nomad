#include "../bff_file/g_bff.h"

void GetScripts(const json& data, bffinfo_t* info)
{
	if (!data.contains("scripts")) {
		return;
	}
	info->numScripts = data["scripts"].size();
	if (info->numScripts >= MAX_SCRIPT_CHUNKS) {
		BFF_Error("WriteBFF: the number of scripts in entries file exceeds the limit of %i scripts", MAX_SCRIPT_CHUNKS);
	}
	memset(info->scripts, 0, sizeof(info->scripts));
	for (int32_t i = 0; i < info->numScripts; i++) {
		bffscript_t* script = &info->scripts[i];
		const std::string scriptnode = "script_"+std::to_string(i);

		if (data["scripts"][scriptnode].contains("name")) {
			const std::string name = data["scripts"][scriptnode]["name"];
			Con_Printf("loading script %s", name.c_str());
			if (name.size() > MAX_BFF_CHUNKNAME - 1) {
				Con_Printf("WARNING: script name at index %lu is greater than %i characters, truncating", i, MAX_BFF_CHUNKNAME - 1);
			}
			strncpy(script->name, name.c_str(), MAX_BFF_CHUNKNAME - 1);
		}
		else {
			Con_Printf("WARNING: script at index %lu doesn't have an assigned name, using default name of %s", scriptnode.c_str());
			strncpy(script->name, scriptnode.c_str(), MAX_BFF_CHUNKNAME - 1);
		}
		
		if (!data["scripts"][scriptnode].contains("codefile")) {
			BFF_Error("WriteBFF: a codefile must be provided when using a scripted sequence in a bff");
		}
		const std::string codefile = data["scripts"][scriptnode]["codefile"];
		
		FILE* fp = SafeOpen(codefile.c_str(), "rb");
		LoadFile(fp, (void **)&script->bytecode, &script->codelen);
		fclose(fp);
	}
}