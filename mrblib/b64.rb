class B64
  def self._b64size(input_size)
    code_padded_size = ((input_size + ( (input_size % 3) ? (3 - (input_size % 3)) : 0) ) / 3) * 4
    newline_size = ((code_padded_size) / 72) * 2;
    Integer(code_padded_size + newline_size + 1)
  end
end
