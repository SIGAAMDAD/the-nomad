typedef int button_t;
#include "src/n_shared.h"
Console con;
std::shared_ptr<spdlog::logger> Log::m_Instance;
#define _G_GAME_
#include "src/g_zone.h"
#include "src/g_zone.cpp"


bool sdl_on = false;
void Log::Init()
{
	std::vector<spdlog::sink_ptr> logSinks;
	logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Files/debug/debug.log", true));

	logSinks[0]->set_pattern("%^[%l]: %v%$");
	logSinks[1]->set_pattern("[%l]: %v");
	
    m_Instance = std::make_shared<spdlog::logger>("LOG", begin(logSinks), end(logSinks));
	spdlog::register_logger(m_Instance);
	m_Instance->set_level(spdlog::level::trace);
	m_Instance->flush_on(spdlog::level::trace);
}

void N_Error(const char *err, ...)
{
    char msg[1024];
    N_memset(msg, 0, sizeof(msg));
    va_list argptr;
    va_start(argptr, err);
    vsnprintf(msg, sizeof(msg), err, argptr);
    va_end(argptr);
    con.ConError("\x1b[31mError: %s\x1b[0m", msg);
    con.ConFlush();
    exit(EXIT_FAILURE);
}

void printvec(int *vec, int size)
{
    for (int i = 0; i < size; ++i) {
        con.ConPrintf("vec[%i]: %i", i, vec[i]);
    }
}

int main(int argc, char** argv)
{
    Log::Init();
    Z_Init();

    int *vec = (int *)Z_Malloc(sizeof(int) * 4, TAG_STATIC, &vec);
    N_memset(vec, 0, sizeof(int) * 4);
    printvec(vec, 4);
    vec[0] = 25;
    printvec(vec, 4);
    vec = (int *)Z_Realloc(vec, sizeof(int) * 6, &vec);
    N_memset(&vec[4], 0, sizeof(int) * 2);
    vec[5] = 53;
    printvec(vec, 6);
//    Z_Free(vec);
    return 0;
}