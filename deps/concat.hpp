#ifndef CONCAT_HPP_
#define CONCAT_HPP_

#include <sstream>

template<class Ch, class Tr = std::char_traits<Ch> >
void concat_to_stream(std::basic_ostream<Ch, Tr>&) {
}

template<class Ch, class Tr = std::char_traits<Ch>, class H,class... T>
void concat_to_stream(std::basic_ostream<Ch, Tr>& os, H&& head, T&&... tail) {
  os << std::forward<H>(head);
  concat_to_stream(os, std::forward<T>(tail)...);
}

template<class... T>
std::string concat(T&&... vals) {
  std::ostringstream os;
  concat_to_stream(os, std::forward<T>(vals)...);
  return os.str();
}

#endif
