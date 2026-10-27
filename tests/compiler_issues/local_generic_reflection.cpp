template <auto member>
decltype(auto) read(auto& value) {
  return (value.[:member:]);
}
int main() {
  auto check = []<typename T>() {
    struct Local {
      T value;
    };
    Local object{};
    return read<^^Local::value>(object);
  };
  (void)check.template operator()<int>();
  (void)check.template operator()<float>();
}
