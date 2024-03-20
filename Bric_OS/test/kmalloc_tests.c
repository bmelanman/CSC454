/** @file kmalloc_tests.c
 *
 * @brief Kernel Memory Allocation Tests
 *
 * @author Bryce Melander
 * @date Feb-19-2024
 *
 * @copyright (c) 2013 The Android Open Source Project. All rights reserved.
 * (See http://www.apache.org/licenses/LICENSE-2.0 for more details.)
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "tests.h"

/*
 * Ported to Custom C Unit Tests by Bryce Melander, 2024
 */

#include "errno.h"
#include "kmalloc.h"
#include "mmu_driver.h"
#include "printk.h"

#define UNITY_BEGIN() OS_INFO( "Running kmalloc unit tests...\n" )
#define UNITY_END()   OS_INFO( "Unit tests complete!\n" )

#define RUN_TEST( test )                        \
    OS_INFO( "Running test `%s`...\n", #test ); \
    test();                                     \
    OS_INFO( "Test `%s` complete.\n", #test )

#define TEST_FAIL()                    \
    OS_ERROR_HALT( "Test failed!\n" ); \
    return 1

#define TEST_ASSERT_EQUAL_INT( exp, act )                                          \
    if ( ( exp ) != ( act ) )                                                      \
    {                                                                              \
        OS_ERROR_HALT( "Assertion failed: %s (%d) is not %s\n", #act, act, #exp ); \
        return 1;                                                                  \
    }

#define TEST_ASSERT_NOT_EQUAL_INT( exp, act )                                      \
    if ( ( exp ) == ( act ) )                                                      \
    {                                                                              \
        OS_ERROR_HALT( "Assertion failed: %s (%d) is not %s\n", #act, act, #exp ); \
        return 1;                                                                  \
    }

#define TEST_ASSERT_EQUAL_UINT( exp, act )                                          \
    if ( ( exp ) != ( act ) )                                                       \
    {                                                                               \
        OS_ERROR_HALT( "Assertion failed: %s (%lu) is not %s\n", #act, act, #exp ); \
        return 1;                                                                   \
    }

#define TEST_ASSERT_LESS_OR_EQUAL_UINT( exp, act )                                              \
    if ( ( act ) > ( exp ) )                                                                    \
    {                                                                                           \
        OS_ERROR_HALT( "Assertion failed: %s (%u) is not <= %s\n", #act, (uint)( act ), #exp ); \
        return 1;                                                                               \
    }

#define TEST_ASSERT_EQUAL_FLOAT( exp, act )                                        \
    if ( ( exp ) != ( act ) )                                                      \
    {                                                                              \
        OS_ERROR_HALT( "Assertion failed: %s (%f) is not %s\n", #act, act, #exp ); \
        return 1;                                                                  \
    }

#define TEST_ASSERT_ERRNO( exp ) TEST_ASSERT_EQUAL_INT( (int)( exp ), errno )

#define TEST_ASSERT_EQUAL_PTR( exp, act )                                          \
    if ( (uint64_t)( exp ) != (uint64_t)( act ) )                                  \
    {                                                                              \
        OS_ERROR_HALT( "Assertion failed: %s (%p) is not %s\n", #act, act, #exp ); \
        return 1;                                                                  \
    }
#define TEST_ASSERT_NOT_EQUAL_PTR( exp, act )                                      \
    if ( (uint64_t)( exp ) == (uint64_t)( act ) )                                  \
    {                                                                              \
        OS_ERROR_HALT( "Assertion failed: %s (%p) is not %s\n", #act, act, #exp ); \
        return 1;                                                                  \
    }

#define TEST_ASSERT_NULL( exp )     TEST_ASSERT_EQUAL_PTR( NULL, exp )
#define TEST_ASSERT_NOT_NULL( exp ) TEST_ASSERT_NOT_EQUAL_PTR( NULL, exp )

#define MALLOC_USABLE_SIZE( ptr ) GET_HEADER( ptr )->size

#define ALLOC_LEN_64U  64U
#define ALLOC_LEN_128U 128U
#define ALLOC_LEN_192U 192U
#define ALLOC_LEN_256U 256U
#define ALLOC_LEN_512U 512U

#define MAX_LOOPS 200U
#define NOERR     0U

#define TEST_VAL ( (uint8_t)( 0xA5 ) )

size_t i, size;

int free_std( void )
{
    errno = 0;
    void *ptr = kmalloc( ALLOC_LEN_128U );
    TEST_ASSERT_NOT_NULL( ptr );

    kfree( ptr );

    TEST_ASSERT_ERRNO( NOERR );

    return 0;
}

int free_nullptr( void )
{
    errno = 0;
    void *ptr = NULL;
    TEST_ASSERT_NULL( ptr );

    kfree( ptr );

    TEST_ASSERT_ERRNO( NOERR );

    return 0;
}

int free_any( void )
{
    void *ptr = NULL;

    errno = 0;
    ptr = kmalloc( ALLOC_LEN_128U );
    TEST_ASSERT_NOT_NULL( ptr );

    ptr = (void *)( (uintptr_t)ptr + ( ALLOC_LEN_128U >> 1 ) );

    kfree( ptr );  // NOLINT

    TEST_ASSERT_ERRNO( NOERR );

    errno = 0;
    ptr = kmalloc( ALLOC_LEN_128U );
    TEST_ASSERT_NOT_NULL( ptr );

    ptr = (void *)( (uintptr_t)ptr + GET_HEADER( ptr )->size );

    kfree( ptr );

    TEST_ASSERT_ERRNO( NOERR );

    return 0;
}

int free_illegal( void )
{
    void *ptr = NULL;

    errno = 0;
    // Address too low
    ptr = (void *)( (uintptr_t)kbrk( 0 ) - ( 2 * BIN_SIZE ) );
    TEST_ASSERT_NOT_NULL( ptr );

    kfree( ptr );

    TEST_ASSERT_ERRNO( EFAULT );

    errno = 0;
    // Address too high
    ptr = (void *)( (uintptr_t)kbrk( 0 ) + ( 2 * BIN_SIZE ) );
    TEST_ASSERT_NOT_NULL( ptr );

    kfree( ptr );

    TEST_ASSERT_ERRNO( EFAULT );

    return 0;
}

int malloc_std( void )
{
    // Simple malloc test.
    void *ptr1 = kmalloc( ALLOC_LEN_128U );

    TEST_ASSERT_NOT_NULL( ptr1 );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_128U, MALLOC_USABLE_SIZE( ptr1 ) );

    kfree( ptr1 );

    // We should get the same pointer back
    void *ptr2 = kmalloc( ALLOC_LEN_128U );

    TEST_ASSERT_NOT_NULL( ptr2 );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_128U, MALLOC_USABLE_SIZE( ptr2 ) );
    TEST_ASSERT_EQUAL_PTR( ptr1, ptr2 );

    kfree( ptr2 );

    return 0;
}

int malloc_overflow( void )
{
    errno = 0;
    void *ptr = kmalloc( SIZE_MAX );

    TEST_ASSERT_NULL( ptr );
    TEST_ASSERT_ERRNO( ENOMEM );

    return 0;
}

int calloc_std( void )
{
    // Simple calloc test.
    uint8_t *ptr = (uint8_t *)kcalloc( 1, ALLOC_LEN_128U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_128U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_128U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( 0, ptr[i] );
    }

    kfree( ptr );

    return 0;
}

int calloc_illegal( void )
{
    errno = 0;

    void *ptr = kcalloc( -1, ALLOC_LEN_128U );

    TEST_ASSERT_NULL( ptr );
    TEST_ASSERT_ERRNO( ENOMEM );

    return 0;
}

int calloc_overflow( void )
{
    errno = 0;

    void *ptr = kcalloc( 1, SIZE_MAX );

    TEST_ASSERT_NULL( ptr );
    TEST_ASSERT_ERRNO( ENOMEM );

    errno = 0;

    ptr = kcalloc( SIZE_MAX, SIZE_MAX );

    TEST_ASSERT_NULL( ptr );
    TEST_ASSERT_ERRNO( ENOMEM );

    errno = 0;

    ptr = kcalloc( 2, SIZE_MAX );

    TEST_ASSERT_NULL( ptr );
    TEST_ASSERT_ERRNO( ENOMEM );

    errno = 0;

    ptr = kcalloc( SIZE_MAX, 2 );

    TEST_ASSERT_NULL( ptr );
    TEST_ASSERT_ERRNO( ENOMEM );

    return 0;
}

int malloc_realloc_smaller( void )
{
    // Realloc to a smaller size, malloc is used for the original allocation.
    uint8_t *ptr = (uint8_t *)kmalloc( ALLOC_LEN_256U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_256U, MALLOC_USABLE_SIZE( ptr ) );

    memset( ptr, TEST_VAL, ALLOC_LEN_256U );

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_128U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_128U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_128U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( TEST_VAL, ptr[i] );
    }

    kfree( ptr );

    return 0;
}

int malloc_realloc_larger( void )
{
    // Realloc to a larger size, malloc is used for the original allocation.
    uint8_t *ptr = (uint8_t *)kmalloc( ALLOC_LEN_128U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_128U, MALLOC_USABLE_SIZE( ptr ) );

    memset( ptr, TEST_VAL, ALLOC_LEN_128U );

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_256U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_256U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_128U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( TEST_VAL, ptr[i] );
    }

    kfree( ptr );

    return 0;
}

int malloc_multiple_realloc( void )
{
    // Multiple reallocs, malloc is used for the original allocation.
    uint8_t *ptr = (uint8_t *)kmalloc( ALLOC_LEN_256U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_256U, MALLOC_USABLE_SIZE( ptr ) );

    memset( ptr, TEST_VAL, ALLOC_LEN_256U );

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_128U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_128U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_128U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( TEST_VAL, ptr[i] );
    }

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_64U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_64U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_64U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( TEST_VAL, ptr[i] );
    }

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_192U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_192U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < 50; i++ )
    {
        TEST_ASSERT_EQUAL_INT( TEST_VAL, ptr[i] );
    }

    memset( ptr, TEST_VAL, ALLOC_LEN_192U );

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_512U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_512U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_192U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( TEST_VAL, ptr[i] );
    }

    kfree( ptr );

    return 0;
}

int calloc_realloc_smaller( void )
{
    // Realloc to a smaller size, calloc is used for the original allocation.
    uint8_t *ptr = (uint8_t *)kcalloc( 1, ALLOC_LEN_256U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_256U, MALLOC_USABLE_SIZE( ptr ) );

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_128U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_128U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_128U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( 0, ptr[i] );
    }

    kfree( ptr );

    return 0;
}

int calloc_realloc_larger( void )
{
    // Realloc to a larger size, calloc is used for the original allocation.
    uint8_t *ptr = (uint8_t *)kcalloc( 1, ALLOC_LEN_128U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_128U, MALLOC_USABLE_SIZE( ptr ) );

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_256U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_256U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_128U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( 0, ptr[i] );
    }

    kfree( ptr );

    return 0;
}

int calloc_multiple_realloc( void )
{
    // Multiple reallocs, calloc is used for the original allocation.
    uint8_t *ptr = (uint8_t *)kcalloc( 1, ALLOC_LEN_256U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_256U, MALLOC_USABLE_SIZE( ptr ) );

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_128U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_128U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_128U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( 0, ptr[i] );
    }

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_64U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_64U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_64U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( 0, ptr[i] );
    }

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_192U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_192U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_64U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( 0, ptr[i] );
    }

    ptr = (uint8_t *)krealloc( ptr, ALLOC_LEN_512U );

    TEST_ASSERT_NOT_NULL( ptr );
    TEST_ASSERT_LESS_OR_EQUAL_UINT( ALLOC_LEN_512U, MALLOC_USABLE_SIZE( ptr ) );
    for ( i = 0; i < ALLOC_LEN_64U; i++ )
    {
        TEST_ASSERT_EQUAL_INT( 0, ptr[i] );
    }

    kfree( ptr );

    return 0;
}

int realloc_overflow( void )
{
    errno = 0;

    TEST_ASSERT_NULL( krealloc( NULL, SIZE_MAX ) );
    TEST_ASSERT_ERRNO( ENOMEM );

    void *ptr = kmalloc( ALLOC_LEN_128U );
    TEST_ASSERT_NOT_NULL( ptr );

    errno = 0;

    TEST_ASSERT_NULL( krealloc( ptr, SIZE_MAX ) );
    TEST_ASSERT_ERRNO( ENOMEM );

    kfree( ptr );

    return 0;
}

int calloc_usable_size( void )
{
    // calloc should zero out all of the memory.
    for ( size = 1; size <= 2048; size++ )
    {
        void *pointer = kmalloc( size );

        TEST_ASSERT_NOT_NULL( pointer );

        memset( pointer, TEST_VAL, MALLOC_USABLE_SIZE( pointer ) );

        kfree( pointer );

        // We should get a previous pointer that has been set to non-zero.
        // If calloc does not zero out all of the data, this will fail.
        uint8_t *zero_mem = (uint8_t *)( kcalloc( 1, size ) );

        TEST_ASSERT_NOT_NULL( pointer );

        size_t usable_size = MALLOC_USABLE_SIZE( zero_mem );

        for ( i = 0; i < usable_size; i++ )
        {
            if ( zero_mem[i] != 0 )
            {
                OS_ERROR_HALT( "Failed at allocation size %zu at byte %zu", size, i );
                TEST_FAIL();
            }
        }

        kfree( zero_mem );
    }

    return 0;
}

int realloc_usable_size( void )
{
    // realloc should zero out all of the memory.
    for ( size = 1; size <= 2048; size++ )
    {
        void *pointer = kmalloc( size );

        TEST_ASSERT_NOT_NULL( pointer );

        memset( pointer, TEST_VAL, MALLOC_USABLE_SIZE( pointer ) );

        pointer = krealloc( pointer, size );

        TEST_ASSERT_NOT_NULL( pointer );

        size_t usable_size = MALLOC_USABLE_SIZE( pointer );

        for ( i = 0; i < usable_size; i++ )
        {
            if ( ( (uint8_t *)pointer )[i] != TEST_VAL )
            {
                OS_ERROR_HALT( "Failed at allocation size %zu at byte %zu", size, i );
                TEST_FAIL();
            }
        }
        kfree( pointer );
    }

    return 0;
}

int malloc_0( void )
{
    void *p = kmalloc( 0 );
    TEST_ASSERT_NOT_NULL( p );
    kfree( p );

    return 0;
}

int calloc_0_0( void )
{
    void *p = kcalloc( 0, 0 );
    TEST_ASSERT_NOT_NULL( p );
    kfree( p );

    return 0;
}

int calloc_0_1( void )
{
    void *p = kcalloc( 0, 1 );
    TEST_ASSERT_NOT_NULL( p );
    kfree( p );

    return 0;
}

int calloc_1_0( void )
{
    void *p = kcalloc( 1, 0 );
    TEST_ASSERT_NOT_NULL( p );
    kfree( p );

    return 0;
}

int realloc_0( void )
{
    void *p = kmalloc( 100 );
    TEST_ASSERT_NOT_NULL( p );

    // krealloc(p, 0) == kfree(p)
    void *p2 = krealloc( p, 0 );

    TEST_ASSERT_NULL( p2 );

    return 0;
}

int realloc_nullptr_0( void )
{
    // krealloc(nullptr, size) is actually kmalloc(size).
    void *p = krealloc( NULL, 0 );
    TEST_ASSERT_NOT_NULL( p );
    kfree( p );

    return 0;
}

// Ensure memory returned by malloc is aligned to allow for certain data types.
int verify_alignment( void )
{
    uint16_t **values_16 = (uint16_t **)kcalloc( MAX_LOOPS, sizeof( uint16_t * ) );
    uint32_t **values_32 = (uint32_t **)kcalloc( MAX_LOOPS, sizeof( uint32_t * ) );
    uint64_t **values_64 = (uint64_t **)kcalloc( MAX_LOOPS, sizeof( uint64_t * ) );
    // long double **values_ldouble = (long double **)kcalloc( MAX_LOOPS, sizeof( long double * ) );

    // Use filler to attempt to force the allocator to get potentially bad alignments.
    void **filler = (void **)kcalloc( MAX_LOOPS, sizeof( void * ) );

    // Check uint16_t pointers.
    for ( i = 0; i < MAX_LOOPS; i++ )
    {
        filler[i] = kmalloc( 1 );
        TEST_ASSERT_NOT_NULL( filler[i] );
        values_16[i] = (uint16_t *)( kmalloc( sizeof( uint16_t ) ) );
        TEST_ASSERT_NOT_NULL( values_16[i] );
        *values_16[i] = i;
        TEST_ASSERT_EQUAL_UINT( *values_16[i], i );

        if ( (uintptr_t)( values_16[i] ) % ( sizeof( uint16_t ) ) != 0 )
        {
            OS_WARN( "Pointer (%p) is not aligned to uint16_t", values_16[i] );
        }

        kfree( filler[i] );
    }

    // Check uint32_t pointers.
    for ( i = 0; i < MAX_LOOPS; i++ )
    {
        filler[i] = kmalloc( 1 );
        TEST_ASSERT_NOT_NULL( filler[i] );
        values_32[i] = (uint32_t *)( kmalloc( sizeof( uint32_t ) ) );
        TEST_ASSERT_NOT_NULL( values_32[i] );
        *values_32[i] = i;
        TEST_ASSERT_EQUAL_UINT( *values_32[i], i );

        if ( (uintptr_t)( values_32[i] ) % ( sizeof( uint32_t ) ) != 0 )
        {
            OS_WARN( "Pointer (%p) is not aligned to uint32_t", values_32[i] );
        }

        kfree( filler[i] );
    }

    // Check uint64_t pointers.
    for ( i = 0; i < MAX_LOOPS; i++ )
    {
        filler[i] = kmalloc( 1 );
        TEST_ASSERT_NOT_NULL( filler[i] );
        values_64[i] = (uint64_t *)( kmalloc( sizeof( uint64_t ) ) );
        TEST_ASSERT_NOT_NULL( values_64[i] );
        *values_64[i] = 0x1000 + i;
        TEST_ASSERT_EQUAL_UINT( *values_64[i], 0x1000 + i );

        if ( (uintptr_t)( values_64[i] ) % ( sizeof( uint64_t ) ) != 0 )
        {
            OS_WARN( "Pointer (%p) is not aligned to uint64_t", values_64[i] );
        }

        kfree( filler[i] );
    }

    // Disabled until long double is implemented.
    //// Check long double pointers.
    // for ( i = 0; i < MAX_LOOPS; i++ )
    //{
    //     filler[i] = kmalloc( 1 );
    //     TEST_ASSERT_NOT_NULL( filler[i] );
    //     values_ldouble[i] = (long double *)( kmalloc( sizeof( long double ) ) );
    //     TEST_ASSERT_NOT_NULL( values_ldouble[i] );
    //     *values_ldouble[i] = 5.5 + (double)i;
    //     TEST_ASSERT_EQUAL_FLOAT( *values_ldouble[i], 5.5 + (double)i );

    //    if ( (uintptr_t)( values_ldouble[i] ) % ( sizeof( long double ) ) != 0 )
    //    {
    //        OS_WARN( "Pointer (%p) is not aligned to long double", values_ldouble[i] );
    //    }

    //    kfree( filler[i] );
    //}

    for ( i = 0; i < MAX_LOOPS; i++ )
    {
        kfree( values_16[i] );
        kfree( values_32[i] );
        kfree( values_64[i] );
        // kfree( values_ldouble[i] );
    }

    kfree( values_16 );
    kfree( values_32 );
    kfree( values_64 );
    // kfree( values_ldouble );
    kfree( filler );

    return 0;
}

int test_kfree( void )
{
    RUN_TEST( free_std );
    RUN_TEST( free_nullptr );
    RUN_TEST( free_any );
    RUN_TEST( free_illegal );

    return 0;
}

int test_kmalloc( void )
{
    RUN_TEST( malloc_std );

    RUN_TEST( malloc_0 );

    RUN_TEST( malloc_overflow );

    RUN_TEST( malloc_realloc_larger );
    RUN_TEST( malloc_realloc_smaller );
    RUN_TEST( malloc_multiple_realloc );

    return 0;
}

int test_kcalloc( void )
{
    RUN_TEST( calloc_std );

    RUN_TEST( calloc_0_0 );
    RUN_TEST( calloc_0_1 );
    RUN_TEST( calloc_1_0 );

    RUN_TEST( calloc_overflow );

    RUN_TEST( calloc_usable_size );

    RUN_TEST( calloc_realloc_larger );
    RUN_TEST( calloc_realloc_smaller );
    RUN_TEST( calloc_multiple_realloc );

    RUN_TEST( calloc_illegal );

    return 0;
}

int test_krealloc( void )
{
    RUN_TEST( realloc_0 );
    RUN_TEST( realloc_nullptr_0 );

    RUN_TEST( realloc_overflow );

    RUN_TEST( realloc_usable_size );

    return 0;
}

int test_kmalloc_all( void )
{
    UNITY_BEGIN();

    RUN_TEST( test_kfree );

    RUN_TEST( test_kmalloc );

    RUN_TEST( test_kcalloc );

    RUN_TEST( test_krealloc );

    RUN_TEST( verify_alignment );

    UNITY_END();

    return 0;
}