#ifndef CTBEXCEPTION_HPP
#define CTBEXCEPTION_HPP

/*******************************************************************************
 * Copyright 2014 GeoData <geodata@soton.ac.uk>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *******************************************************************************/

/**
 * @file CTBException.hpp
 * @brief This declares and defines the `CTBException` class
 */

#include <stdexcept>

namespace ctb {
  class CTBException;
}

/// This represents a CTB runtime error
class ctb::CTBException:
  public std::runtime_error
{ 
public:
  CTBException(const char *message):
    std::runtime_error(message)
  {}
};

#endif /* CTBEXCEPTION_HPP */
