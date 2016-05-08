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
