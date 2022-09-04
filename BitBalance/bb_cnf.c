#include <stdlib.h>
#include <string.h>
#include "configini.h"
#include "bb_cnf.h"

#define BB_CNF_ERRS_N 12

/* Sets error data. */
static void set_error(int err_code, struct bb_error *perr);

int bb_cnf_get_settings(const char *file_name, struct bb_cnf_data *pcd, struct bb_error *perr)
{
    Config *cfg;
    ConfigRet res;
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
            else
            {
                res = ConfigReadStringM(cfg, "SOURCE", "M_PATH", &pcd->m_path);
                if (res != CONFIG_OK)
                    err_code = (res != CONFIG_ERR_MEMALLOC) ? 7 : 1;
                else
                {
                    res = ConfigReadStringM(cfg, "SOURCE", "D_PATH", &pcd->d_path);
                    if (res != CONFIG_OK)
                        err_code = (res != CONFIG_ERR_MEMALLOC) ? 8 : 1;
                    else
                    {
                        res = ConfigReadStringM(cfg, "SOURCE", "P_PATH", &pcd->p_path);
                        if (res != CONFIG_OK)
                            err_code = (res != CONFIG_ERR_MEMALLOC) ? 9 : 1;
                        else
                        {
                            res = ConfigReadStringM(cfg, "RESULT", "DIR", &pcd->result_dir);
                            if (res != CONFIG_OK)
                                err_code = (res != CONFIG_ERR_MEMALLOC) ? 10 : 1;
                            else
                            {
                                res = ConfigReadStringM(cfg, "RESULT", "CSV_PATH", &pcd->csv_path);
                                if (res != CONFIG_OK)
                                    err_code = (res != CONFIG_ERR_MEMALLOC) ? 11 : 1;
                            }
                        }
                    }
                }
            }
        }

        ConfigFree(cfg);
    }

    if (err_code)
        bb_free_cnf_settings(pcd);

    if (err_code && perr)
        set_error(err_code, perr);
    return err_code;
}

void bb_free_cnf_settings(struct bb_cnf_data *pcd)
{
    if (pcd)
    {
        if (pcd->m_path)
            free(pcd->m_path);
        if (pcd->d_path)
            free(pcd->d_path);
        if (pcd->p_path)
            free(pcd->p_path);
        if (pcd->result_dir)
            free(pcd->result_dir);
        if (pcd->csv_path)
            free(pcd->csv_path);

        memset(pcd, 0, sizeof(struct bb_cnf_data));
    }
}

const char * bb_cnf_error_message(int err_code)
{
    static const char *BB_CNF_ERRS[BB_CNF_ERRS_N] = {   "ОК",                                                           //  0
                                                        "Ошибка выделения памяти",                                      //  1
                                                        "Неизвестная ошибка",                                           //  2
                                                        "Пустой указатель",                                             //  3
                                                        "Не удалось открыть конфигурациоонный файл",                    //  4
                                                        "В конфигурационном файле нет секции SOURCE",                   //  5
                                                        "В конфигурационном файле нет секции RESULT",                   //  6
                                                        "В конфигурационном файле в секции SOURCE нет ключа M_PATH",    //  7
                                                        "В конфигурационном файле в секции SOURCE нет ключа D_PATH",    //  8
                                                        "В конфигурационном файле в секции SOURCE нет ключа P_PATH",    //  9
                                                        "В конфигурационном файле в секции RESULT нет ключа DIR",       // 10
                                                        "В конфигурационном файле в секции RESULT нет ключа CSV_PATH",  // 11
    };  

    if ((err_code < 0) || (err_code >= BB_CNF_ERRS_N))
        err_code = 2;

    return *(BB_CNF_ERRS + err_code);
}

static void set_error(int err_code, struct bb_error *perr)
{
    perr->err_code = err_code;
    perr->err_str = bb_cnf_error_message(err_code);
}