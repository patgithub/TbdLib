/************************FreeBSD license header*****************************
 * Copyright (c) 2013, Wilston Oreo
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***********************************************************************/

#pragma once

#include "tbd/config.h" 

namespace tbd  
{
/**@brief Macro for defining a property
   @detail Should be declared in anonymous namespace 
*/
#define TBD_CONFIG_PROPERTY(var_type,property_name,var_name,var_def)\
  class property_name\
  {\
  public:\
    property_name() : var_name##_(var_def) {} \
    property_name(const var_type& _##var_name) : var_name##_(_##var_name) {}\
    TBD_PROPERTY_REF(var_type,var_name)\
  protected:\
    typedef var_type type;\
    template<typename CONFIG_PATH, typename CONFIG>\
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)\
    {\
      var_type _##var_name = _config.get(_path / CONFIG_PATH(name()),var_def);\
      if (_##var_name == var_name##_)\
      {\
        return false;\
      } else\
      {\
        var_name##_ = _##var_name;\
      }\
      return true;\
    }\
    template<typename CONFIG_PATH, typename CONFIG>\
    void save(const CONFIG_PATH& _path, CONFIG& _config) const\
    {\
      _config.put(_path / CONFIG_PATH(name()),var_name());\
    }\
    \
    char const* name() const { return #property_name; }\
    char const* varname() const { return #var_name; }\
    const var_type& value() const { return var_name(); }\
    var_type& value() { return var_name(); }\
    var_type def() const { return var_def; }\
    template<typename FUNCTOR>\
    void apply(FUNCTOR f)\
    {\
      f(name(),varname(),value(),def());\
    }\
  };

/**@brief Macro for defining an array property
   @detail Should be declared in anonymous namespace 
*/
#define TBD_CONFIG_PROPERTY_ARRAY(var_type,property_name,var_name,...)\
  class property_name\
  {\
  public:\
    property_name(const var_type& _##var_name) : var_name##_(_##var_name) {}\
    property_name() : var_name##_({__VA_ARGS__}) {} \
    TBD_PROPERTY_REF(var_type,var_name)\
  protected:\
    typedef var_type type;\
    \
    template<typename CONFIG_PATH, typename CONFIG>\
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)\
    {\
      var_type _##var_name = _config.get_array(_path / CONFIG_PATH(#var_name),{__VA_ARGS__});\
      if (_##var_name == var_name##_)\
      {\
        return false;\
      } else\
      {\
        var_name##_ = _##var_name;\
      }\
      return true;\
    }\
    template<typename CONFIG_PATH, typename CONFIG>\
    void save(const CONFIG_PATH& _path, CONFIG& _config) const\
    {\
      _config.put_array(_path / CONFIG_PATH(#var_name),var_name());\
    }\
    const var_type& value() const { return var_name(); }\
    var_type& value() { return var_name(); }\
    char const* varname() const { return #var_name; }\
    var_type def() const { return {__VA_ARGS__}; }\
    char const* name() const { return #var_name; }\
    template<typename FUNCTOR>\
    void apply(FUNCTOR f)\
    {\
      f(name(),varname(),value(),def());\
    }\
  };

/// Macro for defining a property set
#define  TBD_PROPERTYSET(name,...)\
  typedef tbd::PropertySet<__VA_ARGS__> name;


  /// A propertie set accepts a number of distinct properties as template parameters
  template<typename ...PROPERTIES> struct PropertySet : PROPERTIES... 
  {
    PropertySet(PROPERTIES&&..._properties) : 
      PROPERTIES(_properties)... {}
  };
  
  template<typename PROPERTY, typename...PROPERTIES>
  struct PropertySet<PROPERTY,PROPERTIES...> : PROPERTY, PropertySet<PROPERTIES...>
  {
    PropertySet() {}

    template<typename ARG, typename...ARGS>
    PropertySet(ARG&& _arg, ARGS&&..._args) : 
      PROPERTY(_arg), 
      PropertySet<PROPERTIES...>(_args...) {}

  protected:
    template<typename FUNCTOR>
    void apply(FUNCTOR f)
    {
      PROPERTY::apply(f);
      PropertySet<PROPERTIES...>::apply(f);
    }

    /// Load the properties from a config, a certain config path given
    template<typename CONFIG_PATH, typename CONFIG>
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)
    {
      bool _updated = false;
      _updated |= PROPERTY::load(_path,_config);
      _updated |= PropertySet<PROPERTIES...>::load(_path,_config);
      return _updated;
    }

    /// Saves the properties to a config
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      PROPERTY::save(_path,_config);
      PropertySet<PROPERTIES...>::save(_path,_config);
    }
  };

  template<typename PROPERTY>
  struct PropertySet<PROPERTY> : PROPERTY
  {
  protected:
    PropertySet() {}

    template<typename ARG>
    PropertySet(ARG _arg) : PROPERTY(_arg) {}

    template<typename FUNCTOR>
    void apply(FUNCTOR f)
    {
      f(PROPERTY::name(),PROPERTY::varname(),PROPERTY::value(),PROPERTY::def());
    }


    /// Load the properties from a config, a certain config path given
    template<typename CONFIG_PATH, typename CONFIG>
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)
    {
      return PROPERTY::load(_path,_config);
    }

    /// Saves the properties to a config
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      PROPERTY::save(_path,_config);
    }
  };

  /// A configurable holds a number of properties and config path
  //template<typename ...PROPERTIES> struct Configurable : PROPERTIES... {};

  template<typename...PROPERTIES>
  struct Configurable : PropertySet<PROPERTIES...>
  {
    typedef PropertySet<PROPERTIES...> propertyset_type;

    Configurable(const std::string& _cfgPath) : cfgPath_(_cfgPath) {}
    
    template<typename...ARGS>
    Configurable(const std::string& _cfgPath, 
                 ARGS&&..._args) : 
      cfgPath_(_cfgPath),
      ARGS(_args)... {}

    /// Load the properties from a config
    template<typename CONFIG>
    void load(const CONFIG& _config)
    {
      typedef typename CONFIG::path_type path_type; 
      propertyset_type::load(path_type(cfgPath()),_config);
    }

    template<typename FUNCTOR>
    void apply(FUNCTOR f)
    {
      propertyset_type::apply(f);
    }

    /// Saves the properties to a config
    template<typename CONFIG>
    void save(CONFIG& _config) const
    {
      typedef typename CONFIG::path_type path_type; 
      propertyset_type::save(path_type(cfgPath()),_config);
    }

    TBD_PROPERTY_REF(std::string,cfgPath)
  };
}
