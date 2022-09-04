#include <stdlib.h>
#include <string.h>
#include "configini.h"
#include "bg_cnf.h"

#define BG_CNF_ERRS_N 14
#define BG_DEFAULT_THREADS_N 2

/*  Sets error data.  */
static void bg_settings_set_error(int err_code, struct bb_error *perr);

int bg_cnf_get_settings(const char *file_name, struct bg_cnf_data *pcd, struct bb_error *perr)
{
    Config *cfg;
    ConfigRet res;
    bool save_png;
    int err_code;
    
    cfg = NULL;
    err_code = 0;

    if (!file_name || !pcd)
        err_code = 3;
    else
    {
        if (ConfigReadFile(file_name, &cfg) != CONFIG_OK)
            err_code = 4;
        else
        {
            if (!ConfigHasSection(cfg, "SOURCE"))
                err_code = 5;
            else if (!ConfigHasSection(cfg, "RESULT"))
                err_code = 6;
            else if (!ConfigHasSection(cfg, "THREADS"))
                err_code = 7;
            else if (!ConfigHasSection(cfg, "FORMATS"))
                err_code = 8;
            else
            {
                res = ConfigReadStringM(cfg, "SOURCE", "S_DIR", &pcd->s_dir);
                if (res != CONFIG_OK)
                    err_code = (res != CONFIG_ERR_MEMALLOC) ? 9 : 1;
                else
                {
                    res = ConfigReadStringM(cfg, "SOURCE", "P_PATH", &pcd->p_path);
                    if (res != CONFIG_OK)
                        err_code = (res != CONFIG_ERR_MEMALLOC) ? 12 : 1;
                    else
                    {
                        res = ConfigReadStringM(cfg, "RESULT", "DIR", &pcd->result_dir);
                        if (res != CONFIG_OK)
                            err_code = (res != CONFIG_ERR_MEMALLOC) ? 13 : 1;
                        else
                        {
                            res = ConfigReadInt(cfg, "THREADS", "N", &pcd->threads_n, BG_DEFAULT_THREADS_N);
                            if (res != CONFIG_OK)
                                err_code = (res != CONFIG_ERR_MEMALLOC) ? 14 : 1;
                            else
                            {
                                if (pcd->threads_n <= 0)
                                    pcd->threads_n = BG_DEFAULT_THREADS_N;

                                res = ConfigReadBool(cfg, "FORMATS", "PNG", &save_png, 1);
                                if (res != CONFIG_OK)
                                    err_code = (res != CONFIG_ERR_MEMALLOC) ? 15 : 1;   
                                else
                                    pcd->save_png = (_Bool) save_png;
                            }
                        }
                    }
                }
            }
        }

        ConfigFree(cfg);
    }

    if (err_code)
        bg_free_cnf_settings(pcd);

    if (err_code && perr)
        bg_settings_set_error(err_code, perr);
    return err_code;
}

void bg_free_cnf_settings(struct bg_cnf_data *pcd)
{
    if (pcd)
    {
        if (pcd->s_dir)
            free(pcd->s_dir);
        if (pcd->p_path)
            free(pcd->p_path);
        if (pcd->result_dir)
            free(pcd->result_dir);

        memset(pcd, 0, sizeof(struct bg_cnf_data));
    }
}

const char * bg_cnf_error_message(int err_code)
{
    static const char *BG_CNF_ERRS[BG_CNF_ERRS_N] = {   "ОК",                                                           //  0
                                                        "Ошибка выделения памяти",                                      //  1
                                                        "Неизвестная ошибка",                                           //  2
                                                        "Пустой указатель",                                             //  3
                                                        "Не удалось открыть конфигурациоонный файл",                    //  4
                                                        "В конфигурационном файле нет секции SOURCE",                   //  5
                                                        "В конфигурационном файле нет секции RESULT",                   //  6
                                                        "В конфигурационном файле нет секции THREADS",                  //  7
                                                        "В конфигурационном файле нет секции FORMATS",                  //  8
                                                        "В конфигурационном файле в секции SOURCE нет ключа S_DIR",     //  9
                                                        "В конфигурационном файле в секции SOURCE нет ключа P_PATH",    // 10
                                                        "В конфигурационном файле в секции RESULT нет ключа DIR",       // 11
                                                        "В конфигурационном файле в секции THREADS нет ключа N",        // 12
                                                        "В конфигурационном файле в секции FORMATS нет ключа PNG",      // 13
    };  

    if ((err_code < 0) || (err_code >= BG_CNF_ERRS_N))
        err_code = 2;

    return *(BG_CNF_ERRS + err_code);
}

static void bg_settings_set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bg_cnf_error_message(err_code);
}