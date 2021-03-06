/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include "encoding.h"


static const char *lxb_filepath_test;


TEST_BEGIN(decode)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    size_t size;
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    lxb_codepoint_t cps_buffer[1024];

    enc_data = lxb_encoding_data(LXB_ENCODING_EUC_KR);
    test_ne(enc_data, NULL);

    /* UTF-8: \x58; Unicode: \x00\x58; Code point: 88 */
    to_update_buffer("\x58");
    test_buffer(test_decode_chunks, 1, 88);
    test_buffer(test_decode_full, 1, 88);

    to_update_buffer("\x80");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\xFF");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x81\x40");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x40);
    test_buffer(test_decode_full, 2, rp_cp, 0x40);

    to_update_buffer("\x81\xFF");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x81\x41");
    test_buffer(test_decode_chunks, 1, 0xAC02);
    test_buffer(test_decode_full, 1, 0xAC02);

    to_update_buffer("\xFD\xFE");
    test_buffer(test_decode_chunks, 1, 0x8A70);
    test_buffer(test_decode_full, 1, 0x8A70);

    to_update_buffer("\xFE\xFE");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\xFD\xFE\xFD\xFE");
    test_buffer(test_decode_chunks, 2, 0x8A70, 0x8A70);
    test_buffer(test_decode_full, 2, 0x8A70, 0x8A70);
}
TEST_END

/* Broken encoding. Prepend to stream test. */
TEST_BEGIN(decode_prepend)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    size_t size;
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    lxb_codepoint_t cps_buffer[1024];

    enc_data = lxb_encoding_data(LXB_ENCODING_EUC_KR);
    test_ne(enc_data, NULL);

    to_update_buffer("\xFE\x41");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x41);
    test_buffer(test_decode_full, 2, rp_cp, 0x41);

    to_update_buffer("\xFE\xFE");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);
}
TEST_END

TEST_BEGIN(decode_map)
{
    size_t line;
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_EUC_KR);

    status = test_encoding_process_file(lxb_filepath_test,
                                        test_decode_process_file,
                                        (void *) enc_data, &line);
    if (status != LXB_STATUS_OK) {
        failed_and_exit(line);
    }
}
TEST_END

TEST_BEGIN(encode_map)
{
    size_t line;
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_EUC_KR);

    status = test_encoding_process_file(lxb_filepath_test,
                                        test_encode_process_file,
                                        (void *) enc_data, &line);
    if (status != LXB_STATUS_OK) {
        failed_and_exit(line);
    }
}
TEST_END

TEST_BEGIN(encode_buffer_check)
{
    lxb_status_t status;
    lxb_char_t ch1, ch2[2];

    lxb_codepoint_t cp;
    const lxb_codepoint_t *cps;
    lxb_encoding_encode_t enctx;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_EUC_KR);

    lxb_encoding_encode_init(&enctx, enc_data, &ch1, sizeof(ch1));

    cp = 0x8A70;

    /* 2 */
    cps = &cp;
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, ch2, 2);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 2);
}
TEST_END

int
main(int argc, const char * argv[])
{
    if (argc != 2) {
        printf("Usage:\n\teuc_kr <filepath>\n");
        return EXIT_FAILURE;
    }

    lxb_filepath_test = argv[1];

    TEST_INIT();

    TEST_ADD(decode);
    TEST_ADD(decode_prepend);
    TEST_ADD(decode_map);
    TEST_ADD(encode_map);
    TEST_ADD(encode_buffer_check);

    TEST_RUN("lexbor/encoding/euc_kr");
    TEST_RELEASE();
}
