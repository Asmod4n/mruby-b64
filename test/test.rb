##
# B64 Test

assert("B64.encode / B64.decode roundtrip") do
  src = "hello world"
  enc = B64.encode(src)
  dec = B64.decode(enc)
  assert_equal "aGVsbG8gd29ybGQ=", enc
  assert_equal src, dec
end

assert("B64 empty string") do
  assert_equal "", B64.encode("")
  assert_equal "", B64.decode("")
end

assert("B64 incremental encoder with update/final") do
  b = B64.new
  b.update("hello ")
  b << "world"
  out = b.final
  assert_equal "aGVsbG8gd29ybGQ=", out
end

assert("B64::Decoder incremental decode") do
  d = B64::Decoder.new
  # split into proper 4-char base64 quanta
  parts = ["aGVs", "bG8g", "d29y", "bGQ="]
  out = parts.map { |p| d.decode(p) }.join
  assert_equal "hello world", out
end


assert("B64::Decoder reset") do
  d = B64::Decoder.new
  d.decode("aGVsbG8=")
  d.reset
  out = d.decode("d29ybGQ=")
  assert_equal "world", out
end

assert("B64::Encoder incremental encode") do
  e = B64::Encoder.new
  part1 = e.encode("hel")
  part2 = e.encode("lo ")
  part3 = e.encode("world")
  final = e.final
  assert_equal "aGVsbG8gd29ybGQ=", part1 + part2 + part3 + final
end
