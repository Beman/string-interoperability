//  boost/interop/string.hpp  ----------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#if !defined(BOOST_INTEROP_STRING_HPP)
# define BOOST_INTEROP_STRING_HPP

#include <boost/interop/string_0x.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/interop/iterator_adapter.hpp>
#include <boost/interop/detail/is_iterator.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/decay.hpp>

/*****************************************************************************************

  TODO

  * Every enable_if/disable_if needs to be changed to this form:
      template<class Char>
        typename boost::enable_if_c<is_character<Char>::value
          && !is_same<Char, value_type>::value,
      basic_string&>::type operator=(Char c)

  * Every std::basic_string function that returns basic_string& needs to be forwarded
    to get the right return type (boost::basic_string&)

*****************************************************************************************/

namespace boost
{
namespace xop  // short for interoperability
{
//--------------------------------------------------------------------------------------//
//                              interoperable string types
//--------------------------------------------------------------------------------------//

template<class charT, class traits = std::char_traits<charT>,
  class Allocator = std::allocator<charT> > class basic_string;

typedef basic_string<char>     string;     // narrow with native encoding
typedef basic_string<wchar_t>  wstring;    // wide with native encoding
typedef basic_string<u8_t>     u8string;   // u8_t with UTF-8 encoding
typedef basic_string<u16_t>    u16string;  // u16_t with UTF-16 encoding
typedef basic_string<u32_t>    u32string;  // u32_t with UTF-32 encoding
    
//--------------------------------------------------------------------------------------//
//                       interoperability trait primary templates
//--------------------------------------------------------------------------------------//

//  specializations are provided at the end of this header; some specializations are
//  required by language rules to come after the defintion of basic_string, but the
//  primary templates must be visable before the definition of basic_string

template<class T> class is_character            : public boost::false_type {};
template<class T> class is_character_container  : public boost::false_type {};
  
//--------------------------------------------------------------------------------------//
//
//                              basic_string definition
//
//  Many functions simply forward to append(), which has numerous overloads to provide
//  much of the core functionality.
//  
//--------------------------------------------------------------------------------------//
  
template<class charT, class traits, class Allocator >
class basic_string : public std::basic_string<charT,traits,Allocator>
{
public:
  typedef typename std::basic_string<charT,traits,Allocator>::traits_type      traits_type;
  typedef typename std::basic_string<charT,traits,Allocator>::value_type       value_type;
  typedef typename std::basic_string<charT,traits,Allocator>::allocator_type   allocator_type;
  typedef typename std::basic_string<charT,traits,Allocator>::size_type        size_type;
  typedef typename std::basic_string<charT,traits,Allocator>::difference_type  difference_type;
  typedef typename std::basic_string<charT,traits,Allocator>::reference        reference;
  typedef typename std::basic_string<charT,traits,Allocator>::const_reference  const_reference;
  typedef typename std::basic_string<charT,traits,Allocator>::pointer          pointer;
  typedef typename std::basic_string<charT,traits,Allocator>::const_pointer    const_pointer;
  typedef typename std::basic_string<charT,traits,Allocator>::iterator         iterator;
  typedef typename std::basic_string<charT,traits,Allocator>::const_iterator   const_iterator;
  typedef typename std::basic_string<charT,traits,Allocator>::reverse_iterator reverse_iterator;
  typedef typename std::basic_string<charT,traits,Allocator>::const_reverse_iterator
                                                                               const_reverse_iterator;

  //  C++11 constructors
  basic_string() {}
  explicit basic_string(const Allocator& a)
    : std::basic_string<charT,traits,Allocator>(a) {}
  basic_string(const basic_string& str)
    : std::basic_string<charT,traits,Allocator>(str) {}
  //basic_string(basic_string&& str) noexcept;
  basic_string(const basic_string& str, size_type pos,
               size_type n = std::basic_string<charT,traits,Allocator>::npos,
               const Allocator& a = Allocator())
    : std::basic_string<charT,traits,Allocator>(str, pos, n, a) {}
  basic_string(const charT* s, size_type n, const Allocator& a = Allocator())
    : std::basic_string<charT,traits,Allocator>(s, n, a) {}
  basic_string(const charT* s, const Allocator& a = Allocator())
    : std::basic_string<charT,traits,Allocator>(s, a) {}
  basic_string(size_type n, charT c, const Allocator& a = Allocator())
    : std::basic_string<charT,traits,Allocator>(n, c, a) {}
  template<class InputIterator>
    basic_string(InputIterator begin, InputIterator end,
                 const Allocator& a = Allocator())
      : std::basic_string<charT,traits,Allocator>(begin, end, a) {}
  //basic_string(initializer_list<charT>, const Allocator& = Allocator());
  basic_string(const basic_string& str, const Allocator& a)
    : std::basic_string<charT,traits,Allocator>(str, a) {}
  // basic_string(basic_string&&, const Allocator&);

  //  interoperability constructors

  // dispatching to append(src) is causing an infinite recursion warning, and then
  // infinite recursion at runtime, in VC++ 2010. My problem or theirs? 
  //template <class Source>
  //basic_string(Source src)
  //{ 
  //  append(src);
  //}

  template<class Ctr>
  basic_string(const Ctr& ctr, typename boost::enable_if<is_character_container<Ctr>,
    void*>::type=0)
  {
    converting_iterator<typename Ctr::const_iterator,
      typename Ctr::value_type, by_range,
      typename std::basic_string<charT,traits,Allocator>::value_type>
        itr(ctr.cbegin(), ctr.cend());
    for (; *itr != static_cast<value_type>(0); ++itr)
      push_back(*itr);
  }

  template<class InputIterator>
  basic_string(InputIterator s,
    typename boost::enable_if<boost::is_iterator<InputIterator>, void*>::type=0)
  {
    converting_iterator<InputIterator,
      typename std::iterator_traits<InputIterator>::value_type, by_null,
      typename std::basic_string<charT,traits,Allocator>::value_type>
        itr(s);
    for (; *itr != static_cast<value_type>(0); ++itr)
      push_back(*itr);
  }

  //  c_str()  -------------------------------------------------------------------------//
  //
  //  having c_str() in place early in the development process simplifies testing
   
  const charT* c_str() const {return std::basic_string<charT,traits,Allocator>::c_str();}

  template <class T>
  converting_iterator<const_iterator, value_type, by_range, T>
  c_str() const
  {
    return converting_iterator<const_iterator,
      value_type, by_range, T>(this->begin(), this->end());
  }

  //  append()  ------------------------------------------------------------------------//

  //  signatures given in the order they appear in the standard [string.modifiers]

  basic_string&  operator+=(const basic_string& str)
  {
    std::basic_string<charT,traits,Allocator>::append(str);\
    return *this;
  }
  basic_string&  operator+=(const charT* s)
  {
    std::basic_string<charT,traits,Allocator>::append(s);
    return *this;
  }
  basic_string&  operator+=(charT c)
  {
    std::basic_string<charT,traits,Allocator>::push_back(c);
    return *this;
  }
//basic_string&  operator+=(initializer_list<charT>);


  basic_string&  append(const basic_string& str)
  { 
    std::basic_string<charT,traits,Allocator>::append(str);
    return *this;
  }
  basic_string&  append(const basic_string& str, size_type pos, size_type n)
  { 
    std::basic_string<charT,traits,Allocator>::append(str, pos, n);
    return *this;
  }
  basic_string&  append(const charT* s, size_type n)
  { 
    std::basic_string<charT,traits,Allocator>::append(s, n);
    return *this;
  }
  basic_string&  append(const charT* s)
  { 
    std::basic_string<charT,traits,Allocator>::append(s);
    return *this;
  }
  basic_string&  append(size_type n, charT c)
  { 
    std::basic_string<charT,traits,Allocator>::append(n, c);
    return *this;
  }

  template <class InputIterator>
    typename boost::enable_if<is_same<
      typename std::iterator_traits<InputIterator>::value_type, value_type>,
        basic_string&>::type
  append(InputIterator first, InputIterator last)
  {
    std::basic_string<charT,traits,Allocator>::append(first, last);
    return *this;
  }

//basic_string&  append(initializer_list<charT>);

  void push_back(value_type c) {std::basic_string<charT,traits,Allocator>::push_back(c);}  

  //  append interoperability signatures

  template<class Ctr>
    typename boost::enable_if<is_character_container<Ctr>,
  basic_string&>::type append(const Ctr& ctr)
  {
    converting_iterator<typename Ctr::const_iterator,
      typename Ctr::value_type, by_range,
      typename std::basic_string<charT,traits,Allocator>::value_type>
        itr(ctr.cbegin(), ctr.cend());
    for (; *itr != static_cast<value_type>(0); ++itr)
      std::basic_string<charT,traits,Allocator>::push_back(*itr);
    return *this;
  }

  template<class InputIterator>
    typename boost::enable_if<boost::is_iterator<InputIterator>,
  basic_string&>::type append(InputIterator s, size_type pos, size_type n)
  {
    converting_iterator<InputIterator,
      typename std::iterator_traits<InputIterator>::value_type, by_size,
      typename std::basic_string<charT,traits,Allocator>::value_type>
      itr(std::advance(s, pos), n);
    for (; *itr != static_cast<value_type>(0); ++itr)
      std::basic_string<charT,traits,Allocator>::push_back(*itr);
    return *this;
  }

  template<class InputIterator>
    typename boost::enable_if<boost::is_iterator<InputIterator>,
  basic_string&>::type append(InputIterator s, size_type n)
  {
    converting_iterator<InputIterator,
      typename std::iterator_traits<InputIterator>::value_type, by_size,
      typename std::basic_string<charT,traits,Allocator>::value_type>
        itr(s, n);
    for (; *itr != static_cast<value_type>(0); ++itr)
      std::basic_string<charT,traits,Allocator>::push_back(*itr);
    return *this;
  }

  template<class InputIterator>
    typename boost::enable_if<boost::is_iterator<InputIterator>,
  basic_string&>::type append(InputIterator s)
  {
    converting_iterator<InputIterator,
      typename std::iterator_traits<InputIterator>::value_type, by_null,
      typename std::basic_string<charT,traits,Allocator>::value_type>
        itr(s);
    for (; *itr != static_cast<value_type>(0); ++itr)
      std::basic_string<charT,traits,Allocator>::push_back(*itr);
    return *this;
  }

  template <class InputIterator>
    typename boost::disable_if<is_same<
      typename std::iterator_traits<InputIterator>::value_type, value_type>,
  basic_string&>::type append(InputIterator first, InputIterator last)
  {
    converting_iterator<InputIterator,
      typename std::iterator_traits<InputIterator>::value_type, by_range,
      typename std::basic_string<charT,traits,Allocator>::value_type>
        itr(first, last);
    for (; *itr != static_cast<value_type>(0); ++itr)
      std::basic_string<charT,traits,Allocator>::push_back(*itr);
    return *this;
  }

  template<class Char>
    typename boost::enable_if_c<is_character<Char>::value
      && !is_same<Char, value_type>::value, void>::type
      push_back(Char c)
  {
    converting_iterator<const Char*,
      Char, by_size,
      typename std::basic_string<charT,traits,Allocator>::value_type>
        itr(&c, 1);
    // one input character can produce several output characters; example UTF-32->UTF-8
    for (; *itr != static_cast<value_type>(0); ++itr)
      std::basic_string<charT,traits,Allocator>::push_back(*itr);
  }

  // assignment operator  --------------------------------------------------------------//

  basic_string& operator=(const basic_string& str)
  {
    std::basic_string<charT,traits,Allocator>::assign(str);
    return *this;
  }
  //basic_string& operator=(basic_string&& str) noexcept;
  basic_string& operator=(const charT* s)
  {
    std::basic_string<charT,traits,Allocator>::assign(s);
    return *this;
  }
  basic_string& operator=(charT c)
  {
    std::basic_string<charT,traits,Allocator>::assign(1, c);
    return *this;
  }
  //basic_string& operator=(initializer_list<charT>);

  //  assignment operator interoperability signatures

  template<class Ctr>
    typename boost::enable_if<is_character_container<Ctr>,
  basic_string&>::type operator=(const Ctr& ctr)
  {
    this->clear();
    return append(ctr);
  }

  template<class InputIterator>
    typename boost::enable_if<boost::is_iterator<InputIterator>,
  basic_string&>::type operator=(InputIterator s)
  {
    this->clear();
    return append(s);
  }

  template<class Char>
    typename boost::enable_if_c<is_character<Char>::value
      && !is_same<Char, value_type>::value,
  basic_string&>::type operator=(Char c)
  {
    this->clear();
    push_back(c);
    return *this;
  }

  // assign  -------------------------------------------------------------------------//

  basic_string& assign(const basic_string& str)
  {
    std::basic_string<charT,traits,Allocator>::assign(str);
    return *this;
  }
  //basic_string& assign(basic_string&& str) noexcept;
  basic_string& assign(const basic_string& str, size_type pos, size_type n)
  {
    std::basic_string<charT,traits,Allocator>::assign(str, pos, n);
    return *this;
  }
  basic_string& assign(const charT* s, size_type n)
  {
    std::basic_string<charT,traits,Allocator>::assign(s, n);
    return *this;
  }
  basic_string& assign(const charT* s)
  {
    std::basic_string<charT,traits,Allocator>::assign(s);
    return *this;
  }
  basic_string& assign(size_type n, charT c)
  {
    std::basic_string<charT,traits,Allocator>::assign(n, c);
    return *this;
  }
  template <class InputIterator>
    typename boost::enable_if<is_same<
      typename std::iterator_traits<InputIterator>::value_type, value_type>,
        basic_string&>::type
  assign(InputIterator first, InputIterator last)
  {
    std::basic_string<charT,traits,Allocator>::assign(first, last);
    return *this;
  }

//basic_string& assign(initializer_list<charT>);

  //  assign interoperability signatures

  template<class Ctr>
    typename boost::enable_if<is_character_container<Ctr>,
  basic_string&>::type assign(const Ctr& ctr)
  {
    this->clear();
    return append(ctr);
  }

  template<class InputIterator>
    typename boost::enable_if<boost::is_iterator<InputIterator>,
  basic_string&>::type assign(InputIterator s, size_type pos, size_type n)
  {
    this->clear();
    return append(s, pos, n);
  }

  template<class InputIterator>
    typename boost::enable_if<boost::is_iterator<InputIterator>,
  basic_string&>::type assign(InputIterator s, size_type n)
  {
    this->clear();
    return append(s, n);
  }

  template<class InputIterator>
    typename boost::enable_if<boost::is_iterator<InputIterator>,
  basic_string&>::type assign(InputIterator s)
  {
    this->clear();
    return append(s);
  }

  template <class InputIterator>
    typename boost::disable_if<is_same<
      typename std::iterator_traits<InputIterator>::value_type, value_type>,
  basic_string&>::type assign(InputIterator first, InputIterator last)
  {
    this->clear();
    return append(first, last);
  }

};  // basic_string

//--------------------------------------------------------------------------------------//
//                       interoperability trait specializations
//--------------------------------------------------------------------------------------//

//  specializations are provided here because some are required by language rules to come
//  after the defintion of basic_string

//---------------------------------  is_character  -------------------------------------//

template<> class is_character<char>         : public boost::true_type {}; 
template<> class is_character<wchar_t>      : public boost::true_type {}; 
template<> class is_character<boost::u8_t>  : public boost::true_type {}; 
template<> class is_character<boost::u16_t> : public boost::true_type {}; 
template<> class is_character<boost::u32_t> : public boost::true_type {};

//-----------------------------  is_character_container  -------------------------------//

template<> class is_character_container<std::string>    : public boost::true_type {}; 
template<> class is_character_container<std::wstring>   : public boost::true_type {}; 
template<> class is_character_container<std::basic_string<u8_t> >
                                                        : public boost::true_type {}; 
template<> class is_character_container<std::u16string> : public boost::true_type {}; 
template<> class is_character_container<std::u32string> : public boost::true_type {};

template<> class is_character_container<boost::xop::string>    : public boost::true_type {}; 
template<> class is_character_container<boost::xop::wstring>   : public boost::true_type {}; 
template<> class is_character_container<boost::xop::u8string>  : public boost::true_type {}; 
template<> class is_character_container<boost::xop::u16string> : public boost::true_type {}; 
template<> class is_character_container<boost::xop::u32string> : public boost::true_type {};

}  // namespace xop
}  // namespace boost

#endif  // BOOST_INTEROP_STRING_HPP