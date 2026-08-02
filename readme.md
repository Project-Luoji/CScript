# CScript

“the whole C++ exception handling thing is fundamentally broken. Itd’s _especially_ broken for kernels.”
—Linus Torvalds

## Plan for development

Cscript will be language that inherits the unique syntax structure of each language: C++'s syntax structure; Java's ```throws``` and a mix of C++ & Java's ```abstract & virutal```; Rust's unique ownership system; Go's ```result, err :=```. Above all, the reading simplicity of Python, yet the performance close to C. 


## How does this work?

See {Insert Document} for more information regarding the foundation of this langauge

## Status

### Development stage

[*] Type system 

[x] Class system

[*] Function

[*] Bin

### Release stage

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

function<int8> main(arr<arr<char>> args){
  Person ts = new {"Dyno", 16, true};
  std::printf<std::str, std::int, std::bool>("I am {}, {} years old. Sigma: {} ", ts.getName(), ts.getAge(), ts.Sigma());
  return 0; 
}
```

### Better const and pointer

```
include io;

function<int8> main(arr<arr<char>> args){
  std::int32 r = 4;
  Const<std::int32> b;
  b = 67; // Can be assigned

  Constexpr<std::int32> y = 67; // HAS TO BE ASSIGNED ON SPOT
  Pointer<std::int> z = &r; // Same as C++, but it's automatically a UNIQUE_PTR
  // Pointer<std::int32> zx = &r; // Warning! This value has been assigned by `Pointer<std::int32> z`
  Pointer<Const<std::int></std::int>> alpha = &x; // Simple basically int* const
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

### Everything is a object (Better than Java)
```
// <IF> is no longer a statement, IT's A OBJECT—specifally, a FUNCTION
Function<std::int64> factorial(std::int32 n) {
    return if<std::int64>(n == 1,
    Function<std::int64>() { return 1; },
    Function<std::int64>() { return n * factorial(n-1)}
    );
  }

```

#### Yes Loops are ALSO Object
```
  Function<int8> main(arr<arr<char>>){
    std::loop = new std::for(new Pointer<std::int32>(0), Function<bool>(std::int32 i) {return i < 10;},
    Function<void>(Pointer<std::int32> num){ 
      this->num += 1;
    },
    Function<void>(Pointer<std::int32> i){
      std::println(i);
    });
  }
  // Yes, A LOT OF stuff JUST to make a for(int i=0; i < 10; i++) std::cout<<i<<std::endl;
```
