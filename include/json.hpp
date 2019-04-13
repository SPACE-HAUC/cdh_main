#ifndef _JSON_H_
#define _JSON_H_

#include <string>
#include <iostream>

class json {
public:
  std::string operator[](std::string key){
    return key;
  }
  static json parse(std::istream &in) {
    return json();
  }
};

#endif /* _JSON_H_ */
