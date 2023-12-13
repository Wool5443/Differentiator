//! @file

#ifndef MIN_MAX_HPP
#define MIN_MAX_HPP

/**
 * @brief Finds max of x, y.
 */
#define max(x, y)                                                                                                   \
({                                                                                                                  \
    __typeof__(x) _tx = x; __typeof__(y) _ty = y;                                                                   \
    _tx > _ty ? _tx : _ty;                                                                                          \
})

/**
 * @brief Finds min of x, y.
 */
#define min(x, y)                                                                                                   \
({                                                                                                                  \
    __typeof__(x) _tx = x; __typeof__(y) _ty = y;                                                                   \
    _tx < _ty ? _tx : _ty;                                                                                          \
})

#endif
