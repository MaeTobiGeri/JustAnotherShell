#include "common.h"
#include "config.h"
#include "execute.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <sys/stat.h>

char shell_prompt[256] = "Yash> ";

#define MAX_ALIASES 100
typedef struct {
    char name[100];
    char value[512];
} Alias;

static Alias shell_aliases[MAX_ALIASES];
static int num_aliases = 0;

char* get_alias(const char* name) {
    for (int i = 0; i < num_aliases; i++) {
        if (strcmp(shell_aliases[i].name, name) == 0) {
            return shell_aliases[i].value;
        }
    }
    return NULL;
}

char* get_dynamic_prompt(void) {
    static char dynamic_prompt[1024];
    char cwd[512];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "unknown");
    }

    char *p = shell_prompt;
    char *d = dynamic_prompt;
    while (*p && (d - dynamic_prompt) < 1023) {
        if (*p == '%' && *(p+1) == 'w') {
            const char *home = getenv("HOME");
            char display_cwd[512];
            strncpy(display_cwd, cwd, 511);
            display_cwd[511] = '\0';
            
            if (home && strncmp(cwd, home, strlen(home)) == 0) {
                snprintf(display_cwd, sizeof(display_cwd), "~%s", cwd + strlen(home));
            }
            
            int len = strlen(display_cwd);
            if ((d - dynamic_prompt) + len < 1023) {
                strcpy(d, display_cwd);
                d += len;
            }
            p += 2;
        } else {
            *d++ = *p++;
        }
    }
    *d = '\0';
    return dynamic_prompt;
}

static int l_add_alias(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    const char *value = luaL_checkstring(L, 2);
    if (name && value) {
        for (int i = 0; i < num_aliases; i++) {
            if (strcmp(shell_aliases[i].name, name) == 0) {
                strncpy(shell_aliases[i].value, value, sizeof(shell_aliases[i].value) - 1);
                shell_aliases[i].value[sizeof(shell_aliases[i].value) - 1] = '\0';
                return 0;
            }
        }
        if (num_aliases < MAX_ALIASES) {
            strncpy(shell_aliases[num_aliases].name, name, sizeof(shell_aliases[num_aliases].name) - 1);
            shell_aliases[num_aliases].name[sizeof(shell_aliases[num_aliases].name) - 1] = '\0';
            strncpy(shell_aliases[num_aliases].value, value, sizeof(shell_aliases[num_aliases].value) - 1);
            shell_aliases[num_aliases].value[sizeof(shell_aliases[num_aliases].value) - 1] = '\0';
            num_aliases++;
        }
    }
    return 0;
}

static int l_set_prompt(lua_State *L) {
    const char *prompt = luaL_checkstring(L, 1);
    if (prompt) {
        strncpy(shell_prompt, prompt, sizeof(shell_prompt) - 1);
        shell_prompt[sizeof(shell_prompt) - 1] = '\0';
    }
    return 0;
}

static int l_run_command(lua_State *L) {
    const char *cmd = luaL_checkstring(L, 1);
    if (cmd) {
        int ret = system(cmd);
        lua_pushinteger(L, ret);
        return 1;
    }
    return 0;
}

void load_config(void) {
    const char *home = getenv("HOME");
    if (!home) return;

    char config_dir[512];
    snprintf(config_dir, sizeof(config_dir), "%s/.config/JustAnotherShell", home);

    mkdir(config_dir, 0755);

    char config_file[1024];
    snprintf(config_file, sizeof(config_file), "%s/init.lua", config_dir);

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    lua_newtable(L);
    
    lua_pushcfunction(L, l_set_prompt);
    lua_setfield(L, -2, "set_prompt");

    lua_pushcfunction(L, l_add_alias);
    lua_setfield(L, -2, "add_alias");

    lua_pushcfunction(L, l_run_command);
    lua_setfield(L, -2, "run_command");

    lua_setglobal(L, "yash");

    FILE *f = fopen(config_file, "r");
    if (f) {
        fclose(f);
        if (luaL_dofile(L, config_file) != LUA_OK) {
            fprintf(stderr, "Error loading config: %s\n", lua_tostring(L, -1));
        }
    } else {
        f = fopen(config_file, "w");
        if (f) {
            fprintf(f, "-- JustAnotherShell initialization\n");
            fprintf(f, "-- Split into two parts: general config and scripts\n\n");
            fprintf(f, "local config_dir = os.getenv('HOME') .. '/.config/JustAnotherShell/'\n\n");
            fprintf(f, "-- 1. Configure general features like colors (config.lua)\n");
            fprintf(f, "local config_status, err = pcall(dofile, config_dir .. 'config.lua')\n");
            fprintf(f, "if not config_status and err then\n");
            fprintf(f, "    -- If config.lua doesn't exist, we can ignore or print\n");
            fprintf(f, "end\n\n");
            fprintf(f, "-- 2. Run specific scripts (scripts.lua)\n");
            fprintf(f, "local scripts_status, err = pcall(dofile, config_dir .. 'scripts.lua')\n");
            fprintf(f, "if not scripts_status and err then\n");
            fprintf(f, "    -- Ignore if not found\n");
            fprintf(f, "end\n");
            fclose(f);

            char sub_file[1024];
            snprintf(sub_file, sizeof(sub_file), "%s/config.lua", config_dir);
            FILE *f_conf = fopen(sub_file, "w");
            if (f_conf) {
                fprintf(f_conf, "-- General configurations (colors, prompt, etc.)\n");
                fprintf(f_conf, "-- Examples of color codes:\n");
                fprintf(f_conf, "-- Red: \\1\\27[31m\\2, Green: \\1\\27[32m\\2, Reset: \\1\\27[0m\\2\n");
                fprintf(f_conf, "-- Note: \\1 and \\2 are required around non-printable escape sequences so readline calculates width correctly.\n");
                fprintf(f_conf, "-- You can use %%w in your prompt to show the current working directory.\n");
                fprintf(f_conf, "yash.set_prompt('\\1\\27[32m\\2%%w \\1\\27[36m\\2Yash> \\1\\27[0m\\2')\n");
                fclose(f_conf);
            }

            snprintf(sub_file, sizeof(sub_file), "%s/scripts.lua", config_dir);
            FILE *f_scripts = fopen(sub_file, "w");
            if (f_scripts) {
                fprintf(f_scripts, "-- Run specific scripts or commands here\n");
                fprintf(f_scripts, "-- yash.run_command('echo \"Welcome to JustAnotherShell!\"')\n");
                fclose(f_scripts);
            }

            luaL_dofile(L, config_file);
        }
    }

    lua_close(L);
}
