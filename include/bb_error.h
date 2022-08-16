#ifndef BB_ERROR_
#define BB_ERROR_

struct bb_error {                           // Error data.
    const char *err_str;                    // Error string.
    int err_code;                           // Error code.
};

#endif                                      // BB_ERROR_