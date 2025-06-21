# mruby-b64

Examples
========

```ruby
#taken from https://en.wikipedia.org/wiki/Base64#Examples
plain = "Man is distinguished, not only by his reason, but by this singular passion from
other animals, which is a lust of the mind, that by a perseverance of delight
in the continued and indefatigable generation of knowledge, exceeds the short
vehemence of any carnal pleasure."

base64 = B64.encode(plain)

B64.decode(base64) == plain
```

Streaming Interface for encoding

```ruby
b64 = B64.new

b64 << "welcome " << "to " << "base64 " << "for " << "mruby"

base64 = b64.final

B64.decode(base64) == "welcome to base64 for mruby"
```

API for Streaming Decoding and Encoding
```ruby
encoder = B64::Encoder.new

step1 = encoder.encode "Man is distinguished, not only by his reason, but by this singular passion from\n"
step2 = encoder.encode "other animals, which is a lust of the mind, that by a perseverance of delight\n"
final = encoder.final

B64.decode(step1 + step2 + final) == "Man is distinguished, not only by his reason, but by this singular passion from\nother animals, which is a lust of the mind, that by a perseverance of delight\n"
```

```ruby
decoder = B64::Decoder.new

reverse_step1 = decoder.decode(step1)
reverse_step2_final = decoder.decode(step2 + final)
decoder.reset
"Man is distinguished, not only by his reason, but by this singular passion from\n" + "other animals, which is a lust of the mind, that by a perseverance of delight\n" == reverse_step1 + reverse_step2_final
```