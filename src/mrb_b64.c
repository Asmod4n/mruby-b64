#include <mruby.h>
#include <mruby/string.h>
#include "cencode.h"
#include "cdecode.h"
#include <math.h>
#include <mruby/variable.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/numeric.h>

static mrb_value
mrb_b64_encode(mrb_state *mrb, mrb_value self)
{
  char *input;
  mrb_int input_size;

  mrb_get_args(mrb, "s", &input, &input_size);

  mrb_int code_padded_size = ((input_size + ( (input_size % 3) ? (3 - (input_size % 3)) : 0) ) / 3) * 4;
  mrb_int newline_size = ((code_padded_size) / 72) * 2;

  mrb_int b64_size;
  if (mrb_int_add_overflow(code_padded_size, newline_size + 1, &b64_size)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "base64 input_size doesn't fit into mrb_int");
  }

  mrb_value output_val = mrb_str_new(mrb, NULL, b64_size);

  char *output = RSTRING_PTR(output_val);

  base64_encodestate state;
  base64_init_encodestate(&state);

  int output_size = base64_encode_block(input, input_size, output, &state);
  output += output_size;
  output_size += base64_encode_blockend(output, &state);

  mrb_assert(output_size == RSTRING_LEN(output_val));

  return output_val;
}

static mrb_value
mrb_b64_decode(mrb_state *mrb, mrb_value self)
{
  char *input;
  mrb_int input_size;

  mrb_get_args(mrb, "s", &input, &input_size);

  mrb_value output_val = mrb_str_buf_new(mrb, input_size);

  char *output = RSTRING_PTR(output_val);

  base64_decodestate state;
  base64_init_decodestate(&state);

  int output_size = base64_decode_block(input, input_size, output, &state);

  if (output_size < 0) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "cannot decode block");
  }

  return mrb_str_resize(mrb, output_val, output_size);
}

static const struct mrb_data_type base64_encodestate_type = {
  "$i_mrb_base64_encodestate_type", mrb_free
};

static mrb_value
mrb_b64_init(mrb_state *mrb, mrb_value self)
{
  base64_encodestate *state = (base64_encodestate *) mrb_malloc(mrb, sizeof(base64_encodestate));
  mrb_data_init(self, state, &base64_encodestate_type);
  base64_init_encodestate(state);

  mrb_value buf = mrb_str_buf_new(mrb, 0);

  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "buf"), buf);

  return self;
}

static mrb_value
mrb_b64_update(mrb_state *mrb, mrb_value self)
{
  char *input;
  mrb_int input_size;

  mrb_get_args(mrb, "s", &input, &input_size);

  mrb_value output_val = mrb_str_new(mrb, NULL, ceil((input_size * 4) / 3));

  char *output = RSTRING_PTR(output_val);

  int output_size = base64_encode_block(input, input_size, output, (base64_encodestate *) DATA_PTR(self));

  mrb_str_cat(mrb, mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "buf")), output, output_size);

  return self;
}

static mrb_value
mrb_b64_final(mrb_state *mrb, mrb_value self)
{
  char output[5];

  int output_size = base64_encode_blockend(output, (base64_encodestate *) DATA_PTR(self));

  return mrb_str_cat(mrb, mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "buf")), output, output_size);
}

void
mrb_mruby_b64_gem_init(mrb_state* mrb)
{
  struct RClass* b64_class = mrb_define_class(mrb, "B64", mrb->object_class);
  MRB_SET_INSTANCE_TT(b64_class, MRB_TT_DATA);
  mrb_define_class_method(mrb, b64_class, "encode", mrb_b64_encode, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, b64_class, "decode", mrb_b64_decode, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, b64_class, "initialize", mrb_b64_init, MRB_ARGS_NONE());
  mrb_define_method(mrb, b64_class, "update", mrb_b64_update, MRB_ARGS_REQ(1));
  mrb_define_alias(mrb, b64_class, "<<", "update");
  mrb_define_method(mrb, b64_class, "final", mrb_b64_final, MRB_ARGS_NONE());
}

void mrb_mruby_b64_gem_final(mrb_state* mrb) {}
