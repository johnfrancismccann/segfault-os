#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define NEG_FD -1073741823
#define BIG_FD 1073741823

/* TEST 1 err_read_neg_fd
 * tries to read from a file with file descriptor < 0
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
int err_read_neg_fd(void) {
	uint8_t buf[32];
	if (-1 == ece391_read(NEG_FD, buf, 31)) {
		ece391_fdputs (1, (uint8_t*)"err_read_neg_fd: PASS\n");
		return 0;
	}
	ece391_fdputs (1, (uint8_t*)"err_read_neg_fd: FAIL\n");
	return 2;
}


/* TEST 2 err_write_big_fd
 * tries to write to a file with file descriptor > 7
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
int err_write_big_fd(void) {
	uint8_t buf[32];
    if (-1 == ece391_write(BIG_FD, buf, 31)) {
	    ece391_fdputs (1, (uint8_t*)"err_write_big_fd: PASS\n");
		return 0;
	}
	ece391_fdputs (1, (uint8_t*)"err_write_big_fd: FAIL\n");
	return 2;
}


/* TEST 3 err_open_lots
 * calls open correctly seven times
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
int err_open_lots(void) {
    int32_t i, cnt = 0;
	
	// fd = 0,1 taken, so we should be able to open 6 files (2,3,4,5,6,7)
	// the last file open should fail
    for (i = 0; i < 7; i++) {
	    if (-1 == ece391_open ((uint8_t*)".")) {
			cnt++;
        }
    }
	if (cnt > 1) {
		ece391_fdputs (1, (uint8_t*)"err_open_lots: FAIL\n");
		return 2;
	}
	ece391_fdputs(1, (uint8_t*)"err_open_lots: PASS\n");
	return 0;
}


/* TEST 4 err_open
 * tries to close a file with (2 < fd < 7)
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
int err_open(void) {

	// test with string that matches filename with additional character
	if (-1 == ece391_open ((uint8_t*)"helloo")) {
        ece391_fdputs (1, (uint8_t*)"err_open: attempt 'helloo' PASS\n");
    } else {
        ece391_fdputs (1, (uint8_t*)"err_open: attempt 'helloo' FAIL\n");
		return 2;
	}
	
	// test with string that is short of filename by one character
	if (-1 == ece391_open ((uint8_t*)"shel")) {
        ece391_fdputs (1, (uint8_t*)"err_open: attempt 'shel' PASS\n");
    } else {
        ece391_fdputs (1, (uint8_t*)"err_open: attempt 'shel' FAIL\n");
		return 2;
	}
	
	// test with empty string
	if (-1 == ece391_open ((uint8_t*)"")) {
        ece391_fdputs (1, (uint8_t*)"err_open: attempt '' PASS\n");
    } else {
        ece391_fdputs (1, (uint8_t*)"err_open: attempt '' FAIL\n");
		return 2;
	}
	return 0;
}


/* TEST 5 err_close
 * tries to close a file with (2 < fd < 7)
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
int err_close(void) {
	if (-1 == ece391_close(7)) {
        ece391_fdputs (1, (uint8_t*)"err_close: PASS\n");
		return 0;
	}
	ece391_fdputs (1, (uint8_t*)"err_close: FAIL\n");
	return 2;
}


int main ()
{
	int32_t cnt, select;
    uint8_t buf[128];

    ece391_fdputs (1, (uint8_t*)"Choose from tests 1-5");
    if (-1 == (cnt = ece391_read (0, buf, 127))) {
        ece391_fdputs (1, (uint8_t*)"Can't read test #\n");
		return 2;
    }
	select = (int)(buf[0] - '0');
	
	switch(select) {
		case 1:
			return err_read_neg_fd();
		case 2:
			return err_write_big_fd();
		case 3:
			return err_open_lots();
		case 4:
			return err_open();
		case 5:
			return err_close();
		default:
			ece391_fdputs (1, (uint8_t*)"Invalid test number. Choose from tests 1-5");
			break;
	}
    return 0;
}
