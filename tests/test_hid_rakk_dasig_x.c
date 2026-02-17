// SPDX-License-Identifier: GPL-2.0
/*
 * Userspace unit tests for hid-rakk-dasig-x report_fixup logic.
 *
 * Mocks kernel types and HID infrastructure so the fixup function
 * can be tested in userspace without loading a kernel module.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- Mock kernel types ---- */

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int __u32;

struct hid_device {
  __u16 vendor;
  __u16 product;
};

#define hid_info(hdev, fmt, ...)                                               \
  do {                                                                         \
    (void)(hdev);                                                              \
  } while (0)

/* ---- Pull in the driver constants and function directly ---- */

#define USB_VENDOR_ID_RAKK 0x248A
#define USB_DEVICE_ID_RAKK_DASIG_X 0xFB01

#define RAKK_DASIG_X_RDESC_ORIG_SIZE 193
#define RAKK_DASIG_X_DONGLE_RDESC_ORIG_SIZE 150
#define RAKK_DASIG_X_BLUETOOTH_RDESC_ORIG_SIZE 89
#define RAKK_DASIG_X_USAGE_MAX_OFFSET 17
#define RAKK_DASIG_X_USAGE_MAX_ORIG_VALUE 0x03
#define RAKK_DASIG_X_USAGE_MAX_FIXED_VALUE 0x05

static const __u8 *intercept_dasig_x_report(struct hid_device *hdev,
                                             __u8 *rdesc, unsigned int *rsize) {
  if ((*rsize == RAKK_DASIG_X_RDESC_ORIG_SIZE ||
       *rsize == RAKK_DASIG_X_DONGLE_RDESC_ORIG_SIZE ||
       *rsize == RAKK_DASIG_X_BLUETOOTH_RDESC_ORIG_SIZE) &&
      rdesc[RAKK_DASIG_X_USAGE_MAX_OFFSET] ==
          RAKK_DASIG_X_USAGE_MAX_ORIG_VALUE) {
    hid_info(hdev,
             "Intercepting Rakk Dasig X report descriptor (buttons 3 -> 5)...\n");
    rdesc[RAKK_DASIG_X_USAGE_MAX_OFFSET] = RAKK_DASIG_X_USAGE_MAX_FIXED_VALUE;
  }

  return rdesc;
}

/* ---- Actual report descriptor from the Rakk Dasig X ---- */

static const __u8 dasig_x_original_rdesc[193] = {
    0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x85, 0x01, 0x09, 0x01, 0xa1, 0x00,
    0x05, 0x09, 0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01,
    0x95, 0x05, 0x81, 0x02, 0x75, 0x03, 0x95, 0x01, 0x81, 0x01, 0x05, 0x01,
    0x09, 0x30, 0x09, 0x31, 0x16, 0x01, 0x80, 0x26, 0xff, 0x7f, 0x75, 0x10,
    0x95, 0x02, 0x81, 0x06, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08,
    0x95, 0x01, 0x81, 0x06, 0xc0, 0xc0, 0x05, 0x0c, 0x09, 0x01, 0xa1, 0x01,
    0x85, 0x02, 0x75, 0x10, 0x95, 0x01, 0x15, 0x01, 0x26, 0x8c, 0x02, 0x19,
    0x01, 0x2a, 0x8c, 0x02, 0x81, 0x00, 0xc0, 0x05, 0x01, 0x09, 0x80, 0xa1,
    0x01, 0x85, 0x03, 0x09, 0x82, 0x09, 0x81, 0x09, 0x83, 0x15, 0x00, 0x25,
    0x01, 0x19, 0x01, 0x29, 0x03, 0x75, 0x01, 0x95, 0x03, 0x81, 0x02, 0x95,
    0x05, 0x81, 0x01, 0xc0, 0x05, 0x01, 0x09, 0x00, 0xa1, 0x01, 0x85, 0x05,
    0x15, 0x00, 0x26, 0xff, 0x00, 0x95, 0x20, 0x75, 0x08, 0x09, 0x01, 0x81,
    0x02, 0x09, 0x02, 0x91, 0x02, 0xc0, 0x05, 0x01, 0x09, 0x06, 0xa1, 0x01,
    0x85, 0x06, 0x05, 0x07, 0x19, 0xe0, 0x29, 0xe7, 0x15, 0x00, 0x25, 0x01,
    0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x06, 0x75, 0x08, 0x15, 0x00,
    0x26, 0xff, 0x00, 0x05, 0x07, 0x19, 0x00, 0x2a, 0xff, 0x00, 0x81, 0x00,
    0xc0,
};

/* ---- Actual report descriptor from the Rakk Dasig X (dongle, 150 bytes) ---- */

static const __u8 dasig_x_dongle_rdesc[150] = {
    0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x85, 0x01, 0x09, 0x01, 0xa1, 0x00,
    0x05, 0x09, 0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01,
    0x95, 0x05, 0x81, 0x02, 0x75, 0x03, 0x95, 0x01, 0x81, 0x01, 0x05, 0x01,
    0x09, 0x30, 0x09, 0x31, 0x16, 0x01, 0x80, 0x26, 0xff, 0x7f, 0x75, 0x10,
    0x95, 0x02, 0x81, 0x06, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08,
    0x95, 0x01, 0x81, 0x06, 0xc0, 0xc0, 0x05, 0x0c, 0x09, 0x01, 0xa1, 0x01,
    0x85, 0x02, 0x75, 0x10, 0x95, 0x01, 0x15, 0x01, 0x26, 0x8c, 0x02, 0x19,
    0x01, 0x2a, 0x8c, 0x02, 0x81, 0x00, 0xc0, 0x05, 0x01, 0x09, 0x80, 0xa1,
    0x01, 0x85, 0x03, 0x09, 0x82, 0x09, 0x81, 0x09, 0x83, 0x15, 0x00, 0x25,
    0x01, 0x19, 0x01, 0x29, 0x03, 0x75, 0x01, 0x95, 0x03, 0x81, 0x02, 0x95,
    0x05, 0x81, 0x01, 0xc0, 0x05, 0x01, 0x09, 0x00, 0xa1, 0x01, 0x85, 0x05,
    0x15, 0x00, 0x26, 0xff, 0x00, 0x95, 0x20, 0x75, 0x08, 0x09, 0x01, 0x81,
    0x02, 0x09, 0x02, 0x91, 0x02, 0xc0,
};

/* ---- Actual report descriptor from the Rakk Dasig X (Bluetooth, 89 bytes) ---- */

static const __u8 dasig_x_bluetooth_rdesc[89] = {
    0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x85, 0x03, 0x09, 0x01, 0xa1, 0x00,
    0x05, 0x09, 0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01,
    0x95, 0x05, 0x81, 0x02, 0x75, 0x03, 0x95, 0x01, 0x81, 0x01, 0x05, 0x01,
    0x09, 0x30, 0x09, 0x31, 0x16, 0x01, 0x80, 0x26, 0xff, 0x7f, 0x75, 0x10,
    0x95, 0x02, 0x81, 0x06, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08,
    0x95, 0x01, 0x81, 0x06, 0xc0, 0xc0, 0x06, 0x01, 0xff, 0x09, 0x01, 0xa1,
    0x01, 0x85, 0x05, 0x09, 0x05, 0x15, 0x00, 0x26, 0xff, 0x00, 0x75, 0x08,
    0x95, 0x04, 0xb1, 0x02, 0xc0,
};

/* ---- Test infrastructure ---- */

static int tests_run = 0;
static int tests_passed = 0;

#define RUN_TEST(fn)                                                           \
  do {                                                                         \
    tests_run++;                                                               \
    printf("  %-60s", #fn);                                                    \
    fn();                                                                      \
    tests_passed++;                                                            \
    printf("PASS\n");                                                          \
  } while (0)

/* ---- Test cases ---- */

/* The standard case: descriptor matches size and has the buggy byte. */
static void test_fixup_patches_correct_descriptor(void) {
  __u8 rdesc[193];
  unsigned int rsize = 193;
  struct hid_device hdev = {.vendor = 0x248A, .product = 0xFB01};

  memcpy(rdesc, dasig_x_original_rdesc, sizeof(rdesc));
  assert(rdesc[17] == 0x03);

  const __u8 *ret = intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(ret == rdesc);
  assert(rdesc[17] == 0x05);
}

/* Only byte 17 should change; every other byte must remain intact. */
static void test_fixup_only_modifies_target_byte(void) {
  __u8 rdesc[193];
  __u8 original[193];
  unsigned int rsize = 193;
  struct hid_device hdev = {0};

  memcpy(rdesc, dasig_x_original_rdesc, sizeof(rdesc));
  memcpy(original, dasig_x_original_rdesc, sizeof(original));

  intercept_dasig_x_report(&hdev, rdesc, &rsize);

  for (int i = 0; i < 193; i++) {
    if (i == 17)
      continue;
    assert(rdesc[i] == original[i]);
  }
}

/* Descriptor size doesn't match any known variant -> no patch applied. */
static void test_no_fixup_wrong_size(void) {
  __u8 rdesc[193];
  unsigned int rsize = 100; /* wrong size */
  struct hid_device hdev = {0};

  memcpy(rdesc, dasig_x_original_rdesc, sizeof(rdesc));

  intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(rdesc[17] == 0x03); /* unchanged */
}

/* Byte 17 is already fixed (0x05) -> no double-patch. */
static void test_no_fixup_already_patched(void) {
  __u8 rdesc[193];
  unsigned int rsize = 193;
  struct hid_device hdev = {0};

  memcpy(rdesc, dasig_x_original_rdesc, sizeof(rdesc));
  rdesc[17] = 0x05; /* pre-patched */

  intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(rdesc[17] == 0x05); /* still 0x05, not clobbered */
}

/* Byte 17 has an unexpected value -> don't touch it. */
static void test_no_fixup_unexpected_byte_value(void) {
  __u8 rdesc[193];
  unsigned int rsize = 193;
  struct hid_device hdev = {0};

  memcpy(rdesc, dasig_x_original_rdesc, sizeof(rdesc));
  rdesc[17] = 0x08; /* some unrelated value */

  intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(rdesc[17] == 0x08); /* unchanged */
}

/* Size is 0 -> must not crash or access out of bounds. */
static void test_no_fixup_zero_size(void) {
  __u8 rdesc[1] = {0};
  unsigned int rsize = 0;
  struct hid_device hdev = {0};

  const __u8 *ret = intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(ret == rdesc);
}

/* Descriptor smaller than the fault offset -> no crash, no patch. */
static void test_no_fixup_small_descriptor(void) {
  __u8 rdesc[10] = {0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x85, 0x01, 0x09, 0x01};
  unsigned int rsize = 10;
  struct hid_device hdev = {0};

  const __u8 *ret = intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(ret == rdesc);
}

/* Larger descriptor with the same byte pattern -> no patch (size mismatch). */
static void test_no_fixup_larger_descriptor(void) {
  __u8 rdesc[256];
  unsigned int rsize = 256;
  struct hid_device hdev = {0};

  memset(rdesc, 0, sizeof(rdesc));
  memcpy(rdesc, dasig_x_original_rdesc, 193);

  intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(rdesc[17] == 0x03); /* unchanged - size didn't match */
}

/* Return value is always the same rdesc pointer passed in. */
static void test_return_value_is_rdesc(void) {
  __u8 rdesc_match[193];
  __u8 rdesc_nomatch[193];
  unsigned int rsize;
  struct hid_device hdev = {0};

  /* Case 1: matching descriptor */
  memcpy(rdesc_match, dasig_x_original_rdesc, sizeof(rdesc_match));
  rsize = 193;
  assert(intercept_dasig_x_report(&hdev, rdesc_match, &rsize) == rdesc_match);

  /* Case 2: non-matching descriptor */
  memcpy(rdesc_nomatch, dasig_x_original_rdesc, sizeof(rdesc_nomatch));
  rsize = 100;
  assert(intercept_dasig_x_report(&hdev, rdesc_nomatch, &rsize) ==
         rdesc_nomatch);
}

/* Descriptor size is rsize, not modified by the fixup. */
static void test_rsize_not_modified(void) {
  __u8 rdesc[193];
  unsigned int rsize = 193;
  struct hid_device hdev = {0};

  memcpy(rdesc, dasig_x_original_rdesc, sizeof(rdesc));

  intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(rsize == 193);
}

/* Dongle descriptor (150 bytes) with buggy byte -> should patch. */
static void test_fixup_patches_dongle_descriptor(void) {
  __u8 rdesc[150];
  unsigned int rsize = 150;
  struct hid_device hdev = {.vendor = 0x248A, .product = 0xFA02};

  memcpy(rdesc, dasig_x_dongle_rdesc, sizeof(rdesc));
  assert(rdesc[17] == 0x03);

  const __u8 *ret = intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(ret == rdesc);
  assert(rdesc[17] == 0x05);
}

/* Dongle descriptor: only byte 17 should change. */
static void test_fixup_dongle_only_modifies_target_byte(void) {
  __u8 rdesc[150];
  __u8 original[150];
  unsigned int rsize = 150;
  struct hid_device hdev = {0};

  memcpy(rdesc, dasig_x_dongle_rdesc, sizeof(rdesc));
  memcpy(original, dasig_x_dongle_rdesc, sizeof(original));

  intercept_dasig_x_report(&hdev, rdesc, &rsize);

  for (int i = 0; i < 150; i++) {
    if (i == 17)
      continue;
    assert(rdesc[i] == original[i]);
  }
}

/* Dongle descriptor already patched -> no double-patch. */
static void test_no_fixup_dongle_already_patched(void) {
  __u8 rdesc[150];
  unsigned int rsize = 150;
  struct hid_device hdev = {0};

  memcpy(rdesc, dasig_x_dongle_rdesc, sizeof(rdesc));
  rdesc[17] = 0x05;

  intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(rdesc[17] == 0x05);
}

/* Bluetooth descriptor (89 bytes) with buggy byte -> should patch. */
static void test_fixup_patches_bluetooth_descriptor(void) {
  __u8 rdesc[89];
  unsigned int rsize = 89;
  struct hid_device hdev = {.vendor = 0x248A, .product = 0x8266};

  memcpy(rdesc, dasig_x_bluetooth_rdesc, sizeof(rdesc));
  assert(rdesc[17] == 0x03);

  const __u8 *ret = intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(ret == rdesc);
  assert(rdesc[17] == 0x05);
}

/* Bluetooth descriptor: only byte 17 should change. */
static void test_fixup_bluetooth_only_modifies_target_byte(void) {
  __u8 rdesc[89];
  __u8 original[89];
  unsigned int rsize = 89;
  struct hid_device hdev = {0};

  memcpy(rdesc, dasig_x_bluetooth_rdesc, sizeof(rdesc));
  memcpy(original, dasig_x_bluetooth_rdesc, sizeof(original));

  intercept_dasig_x_report(&hdev, rdesc, &rsize);

  for (int i = 0; i < 89; i++) {
    if (i == 17)
      continue;
    assert(rdesc[i] == original[i]);
  }
}

/* Bluetooth descriptor already patched -> no double-patch. */
static void test_no_fixup_bluetooth_already_patched(void) {
  __u8 rdesc[89];
  unsigned int rsize = 89;
  struct hid_device hdev = {0};

  memcpy(rdesc, dasig_x_bluetooth_rdesc, sizeof(rdesc));
  rdesc[17] = 0x05;

  intercept_dasig_x_report(&hdev, rdesc, &rsize);

  assert(rdesc[17] == 0x05);
}

/* ---- Main ---- */

int main(void) {
  printf("Running hid-rakk-dasig-x unit tests...\n\n");

  RUN_TEST(test_fixup_patches_correct_descriptor);
  RUN_TEST(test_fixup_only_modifies_target_byte);
  RUN_TEST(test_no_fixup_wrong_size);
  RUN_TEST(test_no_fixup_already_patched);
  RUN_TEST(test_no_fixup_unexpected_byte_value);
  RUN_TEST(test_no_fixup_zero_size);
  RUN_TEST(test_no_fixup_small_descriptor);
  RUN_TEST(test_no_fixup_larger_descriptor);
  RUN_TEST(test_return_value_is_rdesc);
  RUN_TEST(test_rsize_not_modified);
  RUN_TEST(test_fixup_patches_dongle_descriptor);
  RUN_TEST(test_fixup_dongle_only_modifies_target_byte);
  RUN_TEST(test_no_fixup_dongle_already_patched);
  RUN_TEST(test_fixup_patches_bluetooth_descriptor);
  RUN_TEST(test_fixup_bluetooth_only_modifies_target_byte);
  RUN_TEST(test_no_fixup_bluetooth_already_patched);

  printf("\n%d/%d tests passed.\n", tests_passed, tests_run);
  return tests_run - tests_passed;
}
