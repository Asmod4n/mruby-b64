/*
cencoder.c - c source to a base64 encoding algorithm implementation

This is part of the libb64 project, and has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64
*/

#include <b64/cencode.h>

void base64_init_encodestate(base64_encodestate* state_in)
{
    state_in->step           = step_A;
    state_in->result         = 0;
    state_in->stepcount      = 0;
    state_in->chars_per_line = BASE64_CENC_DEFCPL;
}

/*-----------------------------------------------------------------------------
   SELECT THE BEST OVERFLOW CHECK AT COMPILE TIME
-----------------------------------------------------------------------------*/
#if defined(__has_builtin)
  #if __has_builtin(__builtin_add_overflow) && __has_builtin(__builtin_mul_overflow)
    #define ADD_OVERFLOW(a,b, res) __builtin_add_overflow((a),(b),(res))
    #define MUL_OVERFLOW(a,b, res) __builtin_mul_overflow((a),(b),(res))
  #else
    #define ADD_OVERFLOW(a,b, res)                                     \
      ( ((*(res) = (a) + (b))) < (a) )
    #define MUL_OVERFLOW(a,b, res)                                     \
      ( ((a) != 0 && (b) > SIZE_MAX / (a))                             \
          ? (*(res) = (a) * (b), 1)                                    \
          : (*(res) = (a) * (b), 0) )
  #endif
#else
  /* no __has_builtin: assume older compiler, use portable checks */
  #define ADD_OVERFLOW(a,b, res)                                     \
    ( ((*(res) = (a) + (b))) < (a) )
  #define MUL_OVERFLOW(a,b, res)                                     \
    ( ((a) != 0 && (b) > SIZE_MAX / (a))                             \
        ? (*(res) = (a) * (b), 1)                                    \
        : (*(res) = (a) * (b), 0) )
#endif

/*
 * Calculate required length of buffer for Base64 encoding (including
 * room for an extra '\0').  Returns 0 on any overflow or if the
 * trailing NUL wouldn’t fit in size_t.
 */
size_t base64_encode_length(size_t plain_len, base64_encodestate* state_in)
{
    size_t tmp, blocks, out_chars, linebreaks, total;

    /* 1) blocks = (plain_len + 2) / 3; overflow check on plain_len+2 */
    if (ADD_OVERFLOW(plain_len, 2, &tmp))
        return 0;
    blocks = tmp / 3;

    /* 2) out_chars = blocks * 4; overflow check on multiplication */
    if (MUL_OVERFLOW(blocks, 4, &out_chars))
        return 0;

    /* 3) line-breaks? */
    if (out_chars && state_in->chars_per_line) {
        size_t cpl = state_in->chars_per_line;

        /* linebreaks = ((out_chars - 1) / cpl) + 1 */
        linebreaks = ((out_chars - 1) / cpl) + 1;

        /* total = out_chars + linebreaks + 1(NUL) */
        if (ADD_OVERFLOW(out_chars, linebreaks, &tmp)) return 0;
        if (ADD_OVERFLOW(tmp, 1, &total))              return 0;
        return total;
    }

    /* 4) no line-breaks: total = out_chars + 1(NUL) */
    if (ADD_OVERFLOW(out_chars, 1, &total))
        return 0;
    return total;
}

char base64_encode_value(signed char value_in)
{
	static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	if (value_in > 63) return '=';
	return encoding[(int)value_in];
}

#define CHECK_BREAK()				\
do						\
{						\
  if (cpl && state_in->stepcount >= cpl)	\
  {						\
    *codechar++ = '\n';				\
    state_in->stepcount = 0;			\
  }						\
  state_in->stepcount++;			\
} while(0);

size_t base64_encode_block(const void* plaintext_in, const size_t length_in, char* code_out, base64_encodestate* state_in)
{
	const char* plainchar = plaintext_in;
	const char* const plaintextend = plainchar + length_in;
	char* codechar = code_out;
	size_t cpl = state_in->chars_per_line;
	char result;
	char fragment;

	result = state_in->result;

	switch (state_in->step)
	{
		for(;;)
		{
	case step_A:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_A;
				return (size_t)(codechar - code_out);
			}

			CHECK_BREAK();

			fragment = *plainchar++;
			result = (fragment & 0x0fc) >> 2;
			*codechar++ = base64_encode_value(result);
			result = (fragment & 0x003) << 4;
	case step_B:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_B;
				return (size_t)(codechar - code_out);
			}

			CHECK_BREAK();

			fragment = *plainchar++;
			result |= (fragment & 0x0f0) >> 4;
			*codechar++ = base64_encode_value(result);
			result = (fragment & 0x00f) << 2;
	case step_C:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_C;
				return (size_t)(codechar - code_out);
			}

			CHECK_BREAK();

			fragment = *plainchar++;
			result |= (fragment & 0x0c0) >> 6;
			*codechar++ = base64_encode_value(result);

			CHECK_BREAK();

			result = (fragment & 0x03f) >> 0;
			*codechar++ = base64_encode_value(result);
		}
	}
	/* control should not reach here */
	return (size_t) (codechar - code_out);
}

size_t base64_encode_blockend(char* code_out, base64_encodestate* state_in)
{
	char* codechar = code_out;
	size_t cpl = state_in->chars_per_line;

	switch (state_in->step)
	{
	case step_B:
		CHECK_BREAK();
		*codechar++ = base64_encode_value(state_in->result);
		CHECK_BREAK();
		*codechar++ = '=';
		CHECK_BREAK();
		*codechar++ = '=';
		break;
	case step_C:
		CHECK_BREAK();
		*codechar++ = base64_encode_value(state_in->result);
		CHECK_BREAK();
		*codechar++ = '=';
		break;
	case step_A:
		break;
	}
	if(state_in->chars_per_line)
		*codechar++ = '\n';

	return (size_t) (codechar - code_out);
}
