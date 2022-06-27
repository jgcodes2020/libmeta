#include <iostream>
#include <string>
#include <meta/container_print.hpp>
#include <meta/named_tuple.hpp>

using meta::ntuple, meta::nt_arg;
using std::cout;

int main() {
  meta::fixed_string s = "Test";
  
  ntuple<nt_arg<"foo", int>, nt_arg<"bar", std::string>> tuple {
    3, "Hello, world!"};
  cout << tuple << '\n';
  cout << meta::get<"foo">(tuple) << '\n';
}