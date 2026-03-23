# CScript

“the whole C++ exception handling thing is fundamentally broken. It’s _especially_ broken for kernels.”
—Linus Torvalds

# Sad news
The first few version of CScript WILL be written in C++.... Then the rest is f*** it

## Why
As a developer, I love C++'s syntax structure. I believe it's very clean and visually appealing—in my opinion. However, I do understand the pain and suffer of C++
Cscript aims to solve most of these problems.  

## Status
[ ] Lexer
[ ] Parser  
[ ] Interpreter
[ ] VSCode extension

## Syntax Overview
### Hello world
```
include io;

function<int> main(char[][] args){
  std::prints("Hello world");
  return 0;
}
```
### OOP
```
include io;

class Thing{
  private:
  std::str name;
  std::int age;
  std::bool isSigma;
  
  public:
  Thing(std::str name, std::int age, std::bool isSigma){} //AS LONG AS THE VAR NAME MATCHES CONSTRUCTOR WILL AUTO FIND.

  virtual function<void> doSomething(Thing target) throws Exception;

  function<std::str> getName() noexcept{
    return this->name;
    }
  function<std::int> getAge() noexcept{
    return this->age;
    }
  function<std::bool> Sigma() noexcept{
    return this->isSigma;
    }
  }

class Person : Thing{
  public:
  real function<void> doSomething(Thing target) throws Exception{
    if(this == target) return null, Exception("Same person lil bro");
    ... // Too lazy and idk what to do lo
    }
    return null;//, null (Not needed because end of function), AND SO IS THE RETURN BECAUSE IT'S A VOID FUNCTION
  }

function<int> main(char[][] args){
  Person ts = new {"Dyno", 16, true};
  std::printf<std::str, std::int, std::bool>("I am {}, {} years old. Sigma: {} ", ts.getName(), ts.getAge(), ts.Sigma());
  return 0; 
}
```
### Better const and pointer
```
include io;

function<int> main(char[][] args){
  std::int r = 4;
  const std::int x; // Can be assigned ONCE
  x = 10; // VALID CODE
  // x = 67; invalid because it has been assigned

  constexpr std::int y = 67; // HAS TO BE ASSIGNED ON SPOT
  Pointer<std::int> z = &r; // Same as C++, but it's automatically a UNIQUE_PTR
  // Pointer<std::int> zx = &r; // Warning! This value has been assigned by `Pointer<std::int> z`
  Pointer<const std::int> alpha = &x; // Simple basically int* const
}
```
### Better type system
```
// Core types
int8, int16, int32, int64        // int system...
uint8, uint16, uint32, uint64   // unsigned int
sint8, sint16, sint32, sint64   // signed int
fl32, fl64                       // floats, double
bool                             // bool

// Official typedefs
typedef uint8  char;   // C BRO
typedef uint8  byte;   // Allias
typedef fl32   fl;     // Lazy
typedef fl64   double  // Double
```

