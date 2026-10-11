import std;
import serial_xml;

class Account {
 public:
  [[= serial_xml::skip]] int balance() const { return balance_; }
  [[ = serial_xml::setter, = serial_xml::name{"balance"} ]] void deposit_balance(int amount) {
    balance_ = amount;
  }

 private:
  int balance_ = 0;
};

int main() {
  const auto account = serial_xml::from_xml<Account>("<Account><balance>125</balance></Account>");
  std::println("Balance: {}", account.balance());
}
