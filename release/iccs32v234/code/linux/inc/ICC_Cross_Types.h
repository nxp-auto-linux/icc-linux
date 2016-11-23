/**
*   @file    ICC_Cross_Types.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication Cross Architecture Types Definition Helper
*   @details Inter Core Communication Cross Architecture Types Definition Helper
*
*   @addtogroup [ICC]
*   @{
*/
/*==================================================================================================
*   Project              : ICC
*   Platform             : Arm Architecture
*   Peripheral           : 
*   Dependencies         : none
*
*   SW Version           : 0.8.0
*   Build Version        : S32V234_ICC_0.8.0
*
*   (c) Copyright 2016 NXP Semiconductor
*   All Rights Reserved.
==================================================================================================*/
/*==================================================================================================
==================================================================================================*/


#ifndef ICC_CROSS_TYPES_H
#define ICC_CROSS_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif


#define ICC_DEFINE_PTR_MATRIX(type, count) \
typedef type (*(PMatrix_ ## type))[][count]

#define ICC_DEFINE_PTR_VECTOR(type, count) \
typedef type (*(PVector_ ## type))[count]

#define ICC_PTR_MATRIX(type) PMatrix_ ## type

#define ICC_PTR_VECTOR(type) PVector_ ## type

#if (defined(ICC_MIX_32_64_SUPPORT) && !defined(__LP64__) && !defined(_LP64))

#define ICC_CROSS_DEFINE(t) \
union Cross_ ## t { \
    t val; \
    uint64_t placeholder; \
}

#define ICC_CROSS_PTR_DEFINE(t) \
union Cross_Ptr_ ## t { \
    t* val; \
    uint64_t placeholder; \
}

#define ICC_CROSS_PTR_MATRIX_DEFINE(t) \
union Cross_Matrix_ ## t { \
    PMatrix_ ## t val; \
    uint64_t placeholder; \
}

#define ICC_CROSS_PTR_VECTOR_DEFINE(t) \
union Cross_Vector_ ## t { \
    PVector_ ## t val; \
    uint64_t placeholder; \
}

#define ICC_CROSS_DECLARE(t) union Cross_ ## t
#define ICC_CROSS_PTR_DECLARE(t) union Cross_Ptr_ ## t
#define ICC_CROSS_PTR_MATRIX_DECLARE(t) union Cross_Matrix_ ## t
#define ICC_CROSS_PTR_VECTOR_DECLARE(t) union Cross_Vector_ ## t

#define ICC_CROSS_INIT(t) { t }
#define ICC_CROSS_ASSIGN(dest, source) (dest.val = source)
#define ICC_CROSS_ASSIGN_CAST(dest, source) (dest.val = (typeof(dest.val))source)
#define ICC_CROSS_COPY(dest, source) (dest.val = source.val)
#define ICC_CROSS_VALUE_OF(t) (t.val)

#else

#define ICC_CROSS_DEFINE(t)
#define ICC_CROSS_PTR_DEFINE(t)
#define ICC_CROSS_PTR_MATRIX_DEFINE(t)
#define ICC_CROSS_PTR_VECTOR_DEFINE(t)

#define ICC_CROSS_DECLARE(t) t
#define ICC_CROSS_PTR_DECLARE(t) t*
#define ICC_CROSS_PTR_MATRIX_DECLARE(t) PMatrix_ ## t
#define ICC_CROSS_PTR_VECTOR_DECLARE(t) PVector_ ## t

#if (defined(__LP64__) || defined(_LP64))
#define ICC_CROSS_INIT(t) (t)
#else
#define ICC_CROSS_INIT(t) (t), (NULL_PTR)
#endif
#define ICC_CROSS_ASSIGN(dest, source) (dest = source)
#define ICC_CROSS_ASSIGN_CAST(dest, source) (dest = (typeof(dest))source)
#define ICC_CROSS_COPY(dest, source) (dest = source)
#define ICC_CROSS_VALUE_OF(t) (t)

#endif


#ifdef __cplusplus
}
#endif

#endif /* ICC_CROSS_TYPES_H */
