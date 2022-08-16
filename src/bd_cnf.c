#include <stdlib.h>
#include "configini.h"
#include "bd_cnf.h"

#define BD_CNF_ERRS_N 7                     // Number of errors.

/*  Sets error data.  */
static void bd_cnf_set_error(int err_code, struct bb_error *perr);

int bd_cnf_get_settings(const char *file_name, char **pm_path, struct bb_error *perr)
{
    Config *cfg;
    ConfigRet res;
    int err_code;
    
    cfg = NULL;
    err_code = 0;

    if (!file_name || !pm_path)
        err_code = 3;
    else
    {
        *pm_path = NULL;

        if (ConfigReadFile(file_name, &cfg) != CONFIG_OK)
            err_code = 4;
        else
        {
            if (!ConfigHasSection(cfg, "SOURCE"))
                err_code = 5;
            else
            {
                res = ConfigReadStringM(cfg, "SOURCE", "M_PATH", pm_path);
                if (res != CONFIG_OK)
                    err_code = (res != CONFIG_ERR_MEMALLOC) ? 6 : 1;
            }
        }

        if (err_code)
            bd_cnf_free_settings(pm_path);

        ConfigFree(cfg);
    }

    if (err_code && perr)
        bd_cnf_set_error(err_code, perr);
    return err_code;
}

void bd_cnf_free_settings(char **pm_path)
{
    if (pm_path && *pm_path)
    {
        free(*pm_path);
        *pm_path = NULL;
    }
}

const char * bd_cnf_error_message(int err_code)
{
    static const char *BD_CNF_ERRS[BD_CNF_ERRS_N] = {   "ОК",                                                           //  0
                                                        "Ошибка выделения памяти",                                      //  1
                                                        "Неизвестная ошибка",                                           //  2
                                                        "Пустой указатель",                                             //  3
                                                        "Не удалось открыть конфигурациоонный файл",                    //  4
                                                        "В конфигурационном файле нет секции SOURCE",                   //  5
                                                        "В конфигурационном файле в секции SOURCE нет ключа M_PATH",    //  6
    };  

    if ((err_code < 0) || (err_code >= BD_CNF_ERRS_N))
        err_code = 2;

    return *(BD_CNF_ERRS + err_code);
}

static void bd_cnf_set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bd_cnf_error_message(err_code);
}
