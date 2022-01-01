#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "buf_t.h"
#include "buf_t_string.h"
#include "buf_t_stats.h"
#include "buf_t_debug.h"

//extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
int test_buf_t(uint8_t *data, size_t size)
{
	buf_t *buf  = buf_new(size);
	buf_t *buf2;

	if (NULL == buf) {
		return 1;
	}

	if (OK != buf_set_data(buf, data, size, size)) {
		goto err1;
	}

	if (OK != buf_is_valid(buf)) {
		goto err1;
	}

	buf2 = buf_new(size);
	if (NULL == buf2) {
		goto err1;
	}

	if (OK != buf_set_data(buf2, data, size, size)) {
		goto err2;
	}

	if (OK != buf_is_valid(buf2)) {
		goto err2;
	}

	if (OK != buf_free(buf)) {
		goto err1;
	}

	bif_free(buf2);


	return 0;

err2:
	buf_free(buf2);

err1:
	buf_free(buf);
	return 1;
}


int main(int argc, char *argv[])
{
	char input[INPUTSIZE] = {0};
        // Slurp input
        if (read(STDIN_FILENO, input, INPUTSIZE) < 0)
        {
                fprintf(stderr, "Couldn't read stdin.\n");
        }

        int ret = process(input);
        if (ret)
        {
                fprintf(stderr, usage, argv[0]);
        };
        return ret;

}
