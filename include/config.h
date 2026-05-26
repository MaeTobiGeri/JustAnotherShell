#ifndef CONFIG_H
#define CONFIG_H

extern char shell_prompt[256];

void load_config(void);
char* get_alias(const char* name);
char* get_dynamic_prompt(void);

#endif
