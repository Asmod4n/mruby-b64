#include <mruby.h>
#include <mruby/string.h>
#include <b64/cencode.h>
#include <b64/cdecode.h>
#include <mruby/variable.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/presym.h>

static mrb_value
mrb_b64_encode(mrb_state *mrb, mrb_value self)
{
  char *input;
  mrb_int input_size;

  mrb_get_args(mrb, "s", &input, &input_size);

  base64_encodestate state;
  base64_init_encodestate(&state);

  size_t b64_size = base64_encode_length(input_size, &state);
  if (b64_size == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "input size is too large");
  }

  mrb_value output_val = mrb_str_new(mrb, NULL, b64_size - 1);

  char *output = RSTRING_PTR(output_val);

  size_t output_size = base64_encode_block(input, input_size, output, &state);
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

  mrb_value output_val = mrb_str_buf_new(mrb, base64_decode_maxlength(input_size) - 1);

  char *output = RSTRING_PTR(output_val);

  base64_decodestate state;
  base64_init_decodestate(&state);

  size_t output_size = base64_decode_block(input, input_size, output, &state);

  return mrb_str_resize(mrb, output_val, output_size);
}

static const struct mrb_data_type base64_encodestate_type = {
  "$i_mrb_base64_encodestate_type", mrb_free
};

static const struct mrb_data_type base64_decodestate_type = {
  "$i_mrb_base64_decodestate_type", mrb_free
};

static mrb_value
mrb_b64_init_decoder(mrb_state *mrb, mrb_value self)
{
  base64_decodestate *state = (base64_decodestate *) mrb_realloc(mrb, DATA_PTR(self), sizeof(base64_decodestate));
  mrb_data_init(self, state, &base64_decodestate_type);
  base64_init_decodestate(state);

  return self;
}

static mrb_value
mrb_b64_decode_block(mrb_state *mrb, mrb_value self)
{
  char *input;
  mrb_int input_size;

  mrb_get_args(mrb, "s", &input, &input_size);

  base64_decodestate *state = (base64_decodestate *) DATA_PTR(self);

  mrb_value output_val = mrb_str_buf_new(mrb, base64_decode_maxlength(input_size) - 1);
  char *output = RSTRING_PTR(output_val);

  size_t output_size = base64_decode_block(input, input_size, output, state);

  return mrb_str_resize(mrb, output_val, output_size);
}

static mrb_value
mrb_b64_decoder_reset(mrb_state *mrb, mrb_value self)
{
  base64_init_decodestate((base64_decodestate *) DATA_PTR(self));
  return self;
}

static mrb_value
mrb_b64_init_encoder(mrb_state *mrb, mrb_value self)
{
  base64_encodestate *state = (base64_encodestate *) mrb_realloc(mrb, DATA_PTR(self), sizeof(base64_encodestate));
  mrb_data_init(self, state, &base64_encodestate_type);
  base64_init_encodestate(state);

  return self;
}

static mrb_value
mrb_b64_encode_block(mrb_state *mrb, mrb_value self)
{
  char *input;
  mrb_int input_size;

  mrb_get_args(mrb, "s", &input, &input_size);

  base64_encodestate *state = (base64_encodestate *) DATA_PTR(self);

  size_t b64_size = base64_encode_length(input_size, state);
  if (b64_size == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "input size is too large");
  }

  mrb_value output_val = mrb_str_buf_new(mrb, b64_size - 1);
  char *output = RSTRING_PTR(output_val);

  size_t output_size = base64_encode_block(input, input_size, output, state);

  return mrb_str_resize(mrb, output_val, output_size);
}

static mrb_value
mrb_b64_encode_blockend(mrb_state *mrb, mrb_value self)
{
  base64_encodestate *state = (base64_encodestate *) DATA_PTR(self);
  mrb_value buffer = mrb_str_buf_new(mrb, 4);
  size_t output_size = base64_encode_blockend(RSTRING_PTR(buffer), state);
  base64_init_encodestate(state);

  return mrb_str_resize(mrb, buffer, output_size);
}

static mrb_value
mrb_b64_init(mrb_state *mrb, mrb_value self)
{
  base64_encodestate *state = (base64_encodestate *) mrb_realloc(mrb, DATA_PTR(self), sizeof(base64_encodestate));
  mrb_data_init(self, state, &base64_encodestate_type);
  base64_init_encodestate(state);

  mrb_iv_set(mrb, self, MRB_SYM(buf), mrb_str_buf_new(mrb, 0));

  return self;
}

static mrb_value
mrb_b64_update(mrb_state *mrb, mrb_value self)
{
  int arena_index = mrb_gc_arena_save(mrb);
  char *input;
  mrb_int input_size;

  mrb_get_args(mrb, "s", &input, &input_size);

  size_t b64_size = base64_encode_length(input_size, (base64_encodestate *) DATA_PTR(self));
  if (b64_size == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "input size is too large");
  }

  mrb_value output_val = mrb_str_new(mrb, NULL, b64_size);

  char *output = RSTRING_PTR(output_val);

  size_t output_size = base64_encode_block(input, input_size, output, (base64_encodestate *) DATA_PTR(self));

  mrb_str_cat(mrb, mrb_iv_get(mrb, self, MRB_SYM(buf)), output, output_size);
  mrb_gc_arena_restore(mrb, arena_index);

  return self;
}

static mrb_value
mrb_b64_final(mrb_state *mrb, mrb_value self)
{
  base64_encodestate *state = (base64_encodestate *) DATA_PTR(self);
  char output[5];
  size_t output_size = base64_encode_blockend(output, state);

  mrb_value buf = mrb_str_cat(mrb, mrb_iv_get(mrb, self, MRB_SYM(buf)), output, output_size);
  mrb_iv_set(mrb, self, MRB_SYM(buf), mrb_str_buf_new(mrb, 0));
  base64_init_encodestate(state);

  return buf;
}

void
mrb_mruby_b64_gem_init(mrb_state* mrb)
{
  struct RClass *b64_class, *b64_decoder_class, *b64_encoder_class;
  b64_class = mrb_define_class_id(mrb, MRB_SYM(B64), mrb->object_class);
  MRB_SET_INSTANCE_TT(b64_class, MRB_TT_DATA);
  mrb_define_class_method_id(mrb, b64_class, MRB_SYM(encode), mrb_b64_encode, MRB_ARGS_REQ(1));
  mrb_define_class_method_id(mrb, b64_class, MRB_SYM(decode), mrb_b64_decode, MRB_ARGS_REQ(1));
  mrb_define_method_id(mrb, b64_class, MRB_SYM(initialize), mrb_b64_init, MRB_ARGS_NONE());
  mrb_define_method_id(mrb, b64_class, MRB_SYM(update), mrb_b64_update, MRB_ARGS_REQ(1));
  mrb_define_alias_id(mrb, b64_class, MRB_OPSYM(lshift), MRB_SYM(update));
  mrb_define_method_id(mrb, b64_class, MRB_SYM(final), mrb_b64_final, MRB_ARGS_NONE());

  b64_decoder_class = mrb_define_class_under_id(mrb, b64_class, MRB_SYM(Decoder), mrb->object_class);
  MRB_SET_INSTANCE_TT(b64_decoder_class, MRB_TT_DATA);
  mrb_define_method_id(mrb, b64_decoder_class, MRB_SYM(initialize), mrb_b64_init_decoder, MRB_ARGS_NONE());
  mrb_define_method_id(mrb, b64_decoder_class, MRB_SYM(decode), mrb_b64_decode_block, MRB_ARGS_REQ(1));
  mrb_define_method_id(mrb, b64_decoder_class, MRB_SYM(reset), mrb_b64_decoder_reset, MRB_ARGS_NONE());

  b64_encoder_class = mrb_define_class_under_id(mrb, b64_class, MRB_SYM(Encoder), mrb->object_class);
  MRB_SET_INSTANCE_TT(b64_encoder_class, MRB_TT_DATA);
  mrb_define_method_id(mrb, b64_encoder_class, MRB_SYM(initialize), mrb_b64_init_encoder, MRB_ARGS_NONE());
  mrb_define_method_id(mrb, b64_encoder_class, MRB_SYM(encode), mrb_b64_encode_block, MRB_ARGS_REQ(1));
  mrb_define_method_id(mrb, b64_encoder_class, MRB_SYM(final), mrb_b64_encode_blockend, MRB_ARGS_REQ(1));
}

void mrb_mruby_b64_gem_final(mrb_state* mrb) {}
