/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <boost/dynamic_bitset.hpp>
#include <boost/type_traits/has_less.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

#if 1
static int tmp_nest = 0 ;
inline void tmp_indent(int n){ for(;n--;) std::cout << "  " ; }
#define COUT(X) tmp_indent(tmp_nest); std::cout << "/* " << __func__ << ": " #X "=" \
                                                << (X) << " */" << std::endl
#else
#define COUT(X) 
#endif

// ============================================================================
// (Commutative) LinearCombination
// ============================================================================

template <typename BASE, typename RING>
struct LinearCombination : std::map<BASE, RING> {
  RING trace ;
  LinearCombination(): trace(RING(0)) {}
  LinearCombination(const BASE & b, const RING & r = RING(1)): std::map<BASE, RING>({{b,r}}), trace(r) {}
//    std::map<BASE, RING>({std::make_pair(b,r)}), trace(r) {}
//    std::map<BASE, RING>({make_pair(b,r)}), trace(r) {}
  LinearCombination & operator*=(const RING & r);
  LinearCombination & operator+=(const LinearCombination & g);
  LinearCombination & operator-=(const LinearCombination & g);
  inline const LinearCombination operator*(const RING & r) const {
    return LinearCombination(*this) *= r ;
  }
  inline const LinearCombination operator+(const LinearCombination & g) const {
    return LinearCombination(*this) += g ;
  }
  inline const LinearCombination operator-(const LinearCombination & g) const {
    return LinearCombination(*this) -= g ;
  }
  inline const LinearCombination operator+() const {
    return (*this) ;
  }
  inline const LinearCombination operator-() const {
    return LinearCombination() - (*this) ;
  }
  inline const int operator!=(const LinearCombination & g) const {
    return (*this - g).size() ;
  }
  inline const int operator==(const LinearCombination & g) const {
    return !(*this != g) ;
  }
//  inline const operator bool() const { return *this != LinearCombination() ; }

  const void gc   ()       ;
  const int  size () const ;
  const int  asize() const ;
  const LinearCombination Term(int m) const ;
  virtual operator std::string() const ;
} ;

template <typename BASE, typename RING>
const void LinearCombination<BASE, RING>::gc(){
  for (auto i = this->begin(); i != this->end();) {
    auto element = i->second ;
    auto j = i ;
    i++ ;
    if(element == RING(0)) this->erase(j) ;
  }
  return ;
}

template <typename BASE, typename RING>
const int LinearCombination<BASE, RING>::size() const {
  int n = 0 ;
  for (auto i = this->begin(); i != this->end(); i++) {
    auto element = i->second ;
    if(element != RING(0)) n++ ;
  }
  return n ;
}

template <typename BASE, typename RING>
const int LinearCombination<BASE, RING>::asize() const {
  int n = 0 ;
  for (auto i = this->begin(); i != this->end(); i++) {
    auto element = i->second ;
    n++ ;
  }
  return n ;
}

template <typename BASE, typename RING>
const LinearCombination<BASE, RING> LinearCombination<BASE, RING>::Term(int m) const {
  int n = 0 ;
  for (auto i = this->begin(); i != this->end(); i++) {
    auto element = i->second ;
    if(element != RING(0)) {
      if(m == n++) return LinearCombination(i->first, i->second) ;
    }
  }
  return LinearCombination();
}

template <typename BASE, typename RING>
std::ostream & operator<<(std::ostream& out, const LinearCombination<BASE, RING> & g){
  const int debug = 0 ;
  int first_output = 1 ;
  for (auto i = g.begin(); i != g.end(); i++) {
    auto base    = i->first  ;
    auto element = i->second ;
    if(element != RING(0)){
      if(first_output) first_output = 0 ; else out << '+' ;
      out << element << "*" << base ;
    }
  }
  if(first_output) out << (debug?"LinearCombination(0)":"0") ;
  return out ;
}

template <typename BASE, typename RING>
LinearCombination<BASE, RING>::operator std::string() const {
  std::stringstream s ;
  s << *this ;
  return s.str() ;
}

template <typename BASE, typename RING>
inline std::string to_string(const LinearCombination<BASE, RING> & L) { return (std::string)L ; }

template <typename BASE, typename RING>
LinearCombination<BASE, RING> & LinearCombination<BASE, RING>::operator*=(
  const RING & r
){
  trace *= r ;
  for (auto i = this->begin(); i != this->end(); i++) i->second *= r ;
  gc();
  return *this ;
}

template <typename BASE, typename RING>
inline const LinearCombination<BASE, RING> operator*(
  const RING & r,
  const LinearCombination<BASE, RING> & g1
){
  return g1*r ;
}

template <typename BASE, typename RING>
LinearCombination<BASE, RING> & LinearCombination<BASE, RING>::operator+=(
  const LinearCombination<BASE, RING> & g
){
  trace += g.trace ;
  for (auto i = g.begin(); i != g.end(); i++) {
    auto base    = i->first  ;
    auto element = i->second ;
    (*this)[base] += element ;
  }
  gc();
  return *this ;
}

template <typename BASE, typename RING>
LinearCombination<BASE, RING> & LinearCombination<BASE, RING>::operator-=(
  const LinearCombination<BASE, RING> & g
){
  trace -= g.trace ;
  for (auto i = g.begin(); i != g.end(); i++) {
    auto base    = i->first  ;
    auto element = i->second ;
    (*this)[base] -= element ;
  }
  gc();
  return *this ;
}

// ============================================================================
// MultiplicativeLinearCombination
// ============================================================================

template <typename BASE, typename GROUP>
struct MultiplicativeLinearCombination : std::map<BASE, GROUP> {
  GROUP trace ;
  MultiplicativeLinearCombination(): trace(GROUP()) {}
  MultiplicativeLinearCombination(const BASE & b, const GROUP & r): std::map<BASE, GROUP>({{b,r}}), trace(r) {}
  MultiplicativeLinearCombination & operator*=(const MultiplicativeLinearCombination & g);
  MultiplicativeLinearCombination & operator/=(const MultiplicativeLinearCombination & g);
  inline const MultiplicativeLinearCombination operator*(const MultiplicativeLinearCombination & g) const {
    return MultiplicativeLinearCombination(*this) *= g ;
  }
  inline const MultiplicativeLinearCombination operator/(const MultiplicativeLinearCombination & g) const {
    return MultiplicativeLinearCombination(*this) /= g ;
  }
  inline const int operator!=(const MultiplicativeLinearCombination & g) const {
    return (*this / g).size() ;
  }
  inline const int operator==(const MultiplicativeLinearCombination & g) const {
    return !(*this != g) ;
  }
//  inline const operator bool() const { return *this != MultiplicativeLinearCombination() ; }
  const void gc   ()       ;
  const int  size () const ;
  const int  asize() const ;
  const MultiplicativeLinearCombination Factor(int m) const ;
} ;

template <typename BASE, typename GROUP>
const void MultiplicativeLinearCombination<BASE, GROUP>::gc(){
  for (auto i = this->begin(); i != this->end();) {
    auto element = i->second ;
    auto j = i ;
    i++ ;
    if(element == GROUP()) this->erase(j) ;
  }
  return ;
}

template <typename BASE, typename GROUP>
const int MultiplicativeLinearCombination<BASE, GROUP>::size() const {
  int n = 0 ;
  for (auto i = this->begin(); i != this->end(); i++) {
    auto element = i->second ;
    if(element != GROUP()) n++ ;
  }
  return n ;
}

template <typename BASE, typename GROUP>
const int MultiplicativeLinearCombination<BASE, GROUP>::asize() const {
  int n = 0 ;
  for (auto i = this->begin(); i != this->end(); i++) {
    auto element = i->second ;
    n++ ;
  }
  return n ;
}

template <typename BASE, typename GROUP>
const MultiplicativeLinearCombination<BASE, GROUP> MultiplicativeLinearCombination<BASE, GROUP>::Factor(int m) const {
  int n = 0 ;
  for (auto i = this->begin(); i != this->end(); i++) {
    auto element = i->second ;
    if(element != GROUP()) {
      if(m == n++) return MultiplicativeLinearCombination(i->first, i->second) ;
    }
  }
  return MultiplicativeLinearCombination();
}

template <typename BASE, typename GROUP>
std::ostream & operator<<(std::ostream& out, const MultiplicativeLinearCombination<BASE, GROUP> & g){
  int first_output = 1 ;
  const int debug = 0 ;
  for (auto i = g.begin(); i != g.end(); i++) {
    auto base    = i->first  ;
    auto element = i->second ;
    if(element != GROUP()){
      if(first_output) first_output = 0 ; else out << '*' ;
      out << "(" << element << ")^" << base ;
    }
  }
  if(first_output) out << (debug ? "MultiplicativeLinearCombination(1)" : "1") ;
  return out ;
}

template <typename BASE, typename GROUP>
MultiplicativeLinearCombination<BASE, GROUP> & MultiplicativeLinearCombination<BASE, GROUP>::operator*=(
  const MultiplicativeLinearCombination<BASE, GROUP> & g
){
  trace *= g.trace ;
  for (auto i = g.begin(); i != g.end(); i++) {
    auto base    = i->first  ;
    auto element = i->second ;
    (*this)[base] *= element ;
  }
  gc();
  return *this ;
}

template <typename BASE, typename GROUP>
MultiplicativeLinearCombination<BASE, GROUP> & MultiplicativeLinearCombination<BASE, GROUP>::operator/=(
  const MultiplicativeLinearCombination<BASE, GROUP> & g
){
  trace /= g.trace ;
  for (auto i = g.begin(); i != g.end(); i++) {
    auto base    = i->first  ;
    auto element = i->second ;
    (*this)[base] /= element ;
  }
  gc();
  return *this ;
}

// ============================================================================
// ReverseOrder
// ============================================================================
//
// layer to patch the order of variables
//

template <typename VARIABLE>
struct ReverseOrder : VARIABLE {
  ReverseOrder() : VARIABLE() {}
  ReverseOrder(const VARIABLE & v) : VARIABLE(v) {}
  inline const int compare (const ReverseOrder & v) const {return -VARIABLE::compare(v);}
  inline const int operator==(const ReverseOrder & i) const {return compare (i)==0;}
  inline const int operator< (const ReverseOrder & i) const {return compare (i)< 0;}
  inline const int operator> (const ReverseOrder & i) const {return compare (i)> 0;}
  inline const int operator<=(const ReverseOrder & i) const {return compare (i)<=0;}
  inline const int operator>=(const ReverseOrder & i) const {return compare (i)>=0;}
  inline const int operator!=(const ReverseOrder & i) const {return compare (i)!=0;}
//  inline const operator bool() const { return *this != ReverseOrder() ; }
} ;

// ============================================================================
// Symmetric Pairing
// ============================================================================

template <typename GROUP>
struct SymmetricPairing : std::pair<GROUP, GROUP> {
  SymmetricPairing(const GROUP & X, const GROUP & Y) : 
    std::pair<GROUP, GROUP>((X<Y)?X:Y, (X<Y)?Y:X) {}

  SymmetricPairing(const std::string & s) :
    std::pair<GROUP,GROUP>(GROUP(s), GROUP()) {}

  SymmetricPairing(int n) :
    std::pair<GROUP,GROUP>(GROUP(), GROUP()) {}

  SymmetricPairing() :
    std::pair<GROUP,GROUP>(GROUP(), GROUP()) {}

  inline const int compare (const SymmetricPairing & i) const {
    int f = this->second.compare(i.second);
    int result = f ? f : this->first.compare(i.first) ; 
    return result ;
  }
  inline const int operator==(const SymmetricPairing & i) const { return compare (i)==0;}
  inline const int operator< (const SymmetricPairing & i) const { return compare (i)< 0;}
  inline const int operator> (const SymmetricPairing & i) const { return compare (i)> 0;}
  inline const int operator<=(const SymmetricPairing & i) const { return compare (i)<=0;}
  inline const int operator>=(const SymmetricPairing & i) const { return compare (i)>=0;}
  inline const int operator!=(const SymmetricPairing & i) const { return compare (i)!=0;}
//  inline const operator bool() const { return *this != SymmetricPairing() ; }
} ;

template <typename GROUP>
std::ostream & operator<<(std::ostream& out, const SymmetricPairing<GROUP> & x){
  if(x.first == GROUP()) return out << "Bug:Pairing(1)" ;
  else if(x.second == GROUP()) return out << x.first ;
  else return out << "e(" << x.first << "," << x.second << ")" ;
  return out ;
}

// ============================================================================
// Monomial
// ============================================================================

template <typename VARIABLE, typename INDEX>
struct Monomial : LinearCombination<VARIABLE, INDEX> {
  typedef LinearCombination<VARIABLE, INDEX> PARENT ;
  inline Monomial(){}
  inline Monomial(const PARENT   & p) : PARENT(p) {}
  inline Monomial(const VARIABLE & v, const INDEX & i = INDEX(1)) : PARENT(v,i) {}
  inline Monomial & operator^=(const INDEX & n){
    ((PARENT&)(*this)) *= n ;
    return (*this) ;
  }
  inline Monomial & operator*=(const Monomial& m){
//    COUT(m) ;
//    COUT(*this) ;
    ((PARENT&)(*this)) += (PARENT&)(m) ;
//    COUT(*this) ;
    return (*this) ;
  }
  inline Monomial & operator/=(const Monomial& m){
    ((PARENT&)(*this)) -= (PARENT&)(m) ;
    return (*this) ;
  }
  inline const Monomial operator^(const INDEX    & n) const {
    return Monomial(*this) ^= n ;
  }
  inline const Monomial operator*(const Monomial & g) const {
    return Monomial(*this) *= g ;
  }
  inline const Monomial operator/(const Monomial & g) const {
    return Monomial(*this) /= g ;
  }
  const int lex_compare     (const Monomial & g1) const ;
  const int grlex_compare   (const Monomial & g1) const ;
  const int grevlex_compare (const Monomial & g1) const ;
  inline const int compare  (const Monomial & g1) const {return lex_compare(g1);}
  inline const int operator==(const Monomial & m) const {return compare(m)==0;}
  inline const int operator< (const Monomial & m) const {return compare(m)< 0;}
  inline const int operator> (const Monomial & m) const {return compare(m)> 0;}
  inline const int operator<=(const Monomial & m) const {return compare(m)<=0;}
  inline const int operator>=(const Monomial & m) const {return compare(m)>=0;}
  inline const int operator!=(const Monomial & m) const {return compare(m)!=0;}
  inline const Monomial Factor(int m) const { return Monomial(PARENT::Term(m)) ; }

//  inline const operator bool() const { return *this != Monomial() ; }
} ;

template <typename VARIABLE, typename INDEX>
std::ostream & operator<<(std::ostream& out, const Monomial<VARIABLE, INDEX> & m){
  int first_output = 1 ;
  const int debug = 0 ;
  for (auto i = m.begin(); i != m.end(); i++) {
    auto variable = i->first  ;
    auto exponent = i->second ;
    if(exponent != INDEX(0)){
      if(first_output) first_output = 0 ; else out << '*' ;
      out << variable ; if(exponent != INDEX(1)) out << '^' << exponent ;
    }
  }
  if(first_output) out << (debug ? "Monomial(1)":"1") ;
  return out ;
}

// Lexicographic Order
template <typename VARIABLE, typename INDEX>
const int Monomial<VARIABLE, INDEX>::lex_compare(
  const Monomial<VARIABLE, INDEX> & g1
) const {
  int val = 0 ;
  const Monomial & g0 = (*this)/g1 ;
  for(auto i = g0.rbegin() ; i != g0.rend() ; i++) {
    auto variable = i->first  ;
    auto index    = i->second ;
    if(index < INDEX(0)){val = -1 ; break ; }
    if(index > INDEX(0)){val =  1 ; break ; }
  }
  return val ;
}

// Graded Lexicographic Order
template <typename VARIABLE, typename INDEX>
const int Monomial<VARIABLE, INDEX>::grlex_compare(
  const Monomial<VARIABLE, INDEX> & g1
) const {
  int val = 0 ;
  if(this->trace > g1.trace) val =  1 ; else
  if(this->trace < g1.trace) val = -1 ; else
  {
    const Monomial & g0 = (*this)/g1 ;
    for(auto i = g0.rbegin(); i != g0.rend(); i++){
      auto variable = i->first  ;
      auto index    = i->second ;
      if(index < INDEX(0)){val = -1 ; break ; }
      if(index > INDEX(0)){val =  1 ; break ; }
    }
  }
  return val ;
}

// Graded Reverse Lexicographic Order
template <typename VARIABLE, typename INDEX>
const int Monomial<VARIABLE, INDEX>::grevlex_compare(
  const Monomial<VARIABLE, INDEX> & g1
) const {
  int val = 0 ;
  if(this->trace > g1.trace) val =  1 ; else
  if(this->trace < g1.trace) val = -1 ; else
  {
    const Monomial & g0 = (*this)/g1 ;
    for(auto i = g0.begin() ; i != g0.end() ; i++) {
      auto variable = i->first  ;
      auto index    = i->second ;
      if(index < INDEX(0)){val = -1 ; break ; }
      if(index > INDEX(0)){val =  1 ; break ; }
    }
  }
  return val ;
}

// ============================================================================
// Polynomial
// ============================================================================

template <typename VARIABLE, typename INDEX, typename RING>
struct Polynomial : LinearCombination<Monomial<VARIABLE, INDEX>, RING> {

  typedef Monomial          <VARIABLE, INDEX> MONOMIAL ;
  typedef LinearCombination <MONOMIAL, RING > PARENT   ;

  inline Polynomial(){}
  inline Polynomial(const PARENT   & p) : PARENT(p) {}
  inline Polynomial(const MONOMIAL & m) : PARENT(m) {}
  inline Polynomial(const VARIABLE & v, const INDEX & i = INDEX(1)) : PARENT(MONOMIAL(v,i)) {}
  inline Polynomial(const RING & r) : PARENT(MONOMIAL(),r) {}
  template<typename RING2 = RING>
  inline Polynomial(typename boost::disable_if<boost::is_same<RING2, int>, const int>::type i) : PARENT(MONOMIAL(),RING2(i)) {}

  // =============================================================

         Polynomial & operator^=(const INDEX    & n);
         Polynomial & operator*=(const Polynomial & p2);

  // =============================================================

  inline Polynomial & operator*=(const RING       & r){
    ((PARENT&)(*this)) *= r ;
    return (*this) ;
  }
  inline Polynomial & operator+=(const Polynomial & m){
    ((PARENT&)(*this)) += (PARENT&)(m) ;
    return (*this) ;
  }
  inline Polynomial & operator-=(const Polynomial & m){
    ((PARENT&)(*this)) -= (PARENT&)(m) ;
    return (*this) ;
  }

  // =============================================================

  inline const Polynomial operator^(const INDEX    & n) const {
    return Polynomial(*this) ^= n ;
  }
  inline const Polynomial operator*(const Polynomial & p) const {
    return Polynomial(*this) *= p ;
  }
  inline const Polynomial operator*(const RING       & r) const {
    return Polynomial(*this) *= r ;
  }
  inline const Polynomial operator+(const Polynomial & g) const {
    return Polynomial(*this) += g ;
  }
  inline const Polynomial operator-(const Polynomial & g) const {
    return Polynomial(*this) -= g ;
  }
  inline const Polynomial operator+() const {
    return (*this) ;
  }
  inline const Polynomial operator-() const {
    return Polynomial() - (*this) ;
  }
  inline const Polynomial Term(int m) const {
    return Polynomial(PARENT::Term(m)) ;
  }
  template<typename INDEX2 = INDEX>
  inline const int compare (typename boost::disable_if<boost::is_same<INDEX2, long long>, const Polynomial &>::type g) const {
    const Polynomial t = *this - g ;
    if (t.size() == 0) return 0 ;
    else return t.rbegin()->second.compare(RING(0)) ;
  }
  template<typename INDEX2 = INDEX>
  inline const int compare (typename boost::enable_if<boost::is_same<INDEX2, long long>, const Polynomial &>::type g) const {
    const Polynomial t = *this - g ;
    if (t.size() == 0) return 0 ;
    else return t.rbegin()->second ;
  }
  inline const int operator==(const Polynomial & m) const {return compare(m)==0;}
  inline const int operator< (const Polynomial & m) const {return compare(m)< 0;}
  inline const int operator> (const Polynomial & m) const {return compare(m)> 0;}
  inline const int operator<=(const Polynomial & m) const {return compare(m)<=0;}
  inline const int operator>=(const Polynomial & m) const {return compare(m)>=0;}
  inline const int operator!=(const Polynomial & m) const {return compare(m)!=0;}

  inline operator bool() const { return *this != Polynomial() ; }

/*
  inline const int operator!=(const Polynomial & g) const {
    return ((PARENT&)(*this)) != (PARENT&)(g) ;
  }
  inline const int operator==(const Polynomial & g) const {
    return ((PARENT&)(*this)) == (PARENT&)(g) ;
  }
*/
} ;

template <typename VARIABLE, typename INDEX, typename RING>
typename boost::enable_if<boost::has_less<RING>, std::ostream &>::type
operator<<(std::ostream& out, const Polynomial<VARIABLE, INDEX, RING> & p){
  int first_output = 1 ;
  const int debug = 0 ; 
  out << '(' ;
  for (auto i = p.rbegin(); i != p.rend(); i++) {
    auto m = i->first  ;
    auto c = i->second ;
    if(c != RING(0)){
      if(first_output) first_output = 0 ; else if(RING(0) < c) out << '+' ;
      if(m != Monomial<VARIABLE, INDEX>()){
        if(c != RING(1)){
	  if(c == -RING(1)) out << "-" ;
	  else              out << c << "*" ;
        }
        out << m ;
      }else{
        out << c ;
      }
    }
  }
  if(first_output) out << (debug ? "Polynomial(0)" : "0") ;
  out << ')' ;
  return out ;
}

template <typename VARIABLE, typename INDEX, typename RING>
typename boost::disable_if<boost::has_less<RING>, std::ostream &>::type
operator<<(std::ostream& out, const Polynomial<VARIABLE, INDEX, RING> & p){
  int first_output = 1 ;
  const int debug = 0 ; 
  for (auto i = p.rbegin(); i != p.rend(); i++) {
    auto m = i->first  ;
    auto c = i->second ;
    if(c != RING(0)){
      if(first_output) first_output = 0 ; else out << '+' ;
      if(m != Monomial<VARIABLE, INDEX>()){
        if(c != RING(1)) out << c << "*" ;
        out << m ;
      }else{
        out << c ;
      }
    }
  }
  if(first_output) out << (debug ? "Polynomial(0)" : "0") ;
  return out ;
}

template <typename VARIABLE, typename INDEX, typename RING>
Polynomial<VARIABLE, INDEX, RING> & Polynomial<VARIABLE, INDEX, RING>::operator^=(
  const INDEX & n_
){
  typedef unsigned long long         uinteger_t   ;
  typedef boost::dynamic_bitset<uinteger_t> bitvector_t  ;
  INDEX n = n_ ;

  if(n< INDEX(0))throw "negative n in Polynomial pow" ;
  bitvector_t b ;
  while(n != INDEX(0)){
    b.push_back((bool)(n%2));
    n /= 2 ;
  }
  Polynomial p = Polynomial(MONOMIAL()) ;
  for(auto i=(int)b.size()-1;i>=0;i--){
    p*=p ;
    if(b[i])p*=(*this) ;
  }
  p.gc() ;
  return (*this) = p ;
}

template <typename VARIABLE, typename INDEX, typename RING>
Polynomial<VARIABLE, INDEX, RING> & Polynomial<VARIABLE, INDEX, RING>::operator*=(
  const Polynomial<VARIABLE, INDEX, RING> & p2
){
        Polynomial   p0         ;
  const Polynomial & p1 = *this ;
  for (auto i = p1.begin(); i != p1.end(); i++) {
    auto m1 = i->first  ;
    auto c1 = i->second ;
    for (auto j = p2.begin(); j != p2.end(); j++) {
      auto m2 = j->first  ;
      auto c2 = j->second ;
      p0[m1*m2] += c1*c2  ;
    }
  }
  p0.gc() ;
  return *this = p0 ;
}

template <typename VARIABLE, typename INDEX, typename RING>
inline const Polynomial<VARIABLE, INDEX, RING> operator*(
  const RING & r,
  const Polynomial<VARIABLE, INDEX, RING> & g1
){
  return g1*r ;
}

template <typename VARIABLE, typename INDEX, typename RING>
inline const Polynomial<VARIABLE, INDEX, RING> operator+(
  const RING & r,
  const Polynomial<VARIABLE, INDEX, RING> & g1
){
  return g1+r ;
}

template <typename VARIABLE, typename INDEX, typename RING>
inline const Polynomial<VARIABLE, INDEX, RING> operator-(
  const RING & r,
  const Polynomial<VARIABLE, INDEX, RING> & g1
){
  return -g1+r ;
}

template <typename VARIABLE1, typename VARIABLE2, typename INDEX, typename RING>
std::ostream & operator<<(
  std::ostream& out,
  const Monomial<VARIABLE1, Polynomial<VARIABLE2, INDEX, RING> > & m
){
  int first_output = 1 ;
  const int debug = 0 ;
  for (auto i = m.begin(); i != m.end(); i++) {
    auto variable = i->first  ;
    auto exponent = i->second ;
    if(exponent != Polynomial<VARIABLE2, INDEX, RING>(0)){
      if(first_output) first_output = 0 ; else out << '*' ;
      out << variable ;
      if(exponent != Polynomial<VARIABLE2, INDEX, RING>(1)){
        out << '^' ;

	if((exponent.size()==1) && (
	  (exponent.begin()->first.size()==0) || (
	    (exponent.begin()->first.size()==1) && (
	      (exponent.begin()->first.begin()->second ==  RING(1)) &&
	      (exponent.begin()->first.begin()->second == -RING(1))
	    )
	  )
	)) out << exponent ;
	else out << '(' << exponent << ')' ;
      }
    }
  }
  if(first_output) out << (debug ? "Monomial(1)" : "1") ;
  return out ;
}

#endif /* POLYNOMIAL_H */
