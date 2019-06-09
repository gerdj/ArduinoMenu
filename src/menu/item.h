/* -*- C++ -*- */
#pragma once

#include "base.h"

/** \defgroup Agents Command and navigation agents
 *  @{
 */

/**
* The CmdAgent class represents an item that might receive navigation commands
*/
struct CmdAgent {
  virtual bool canNav() const =0;
  virtual bool up(void* o)=0;
  virtual bool down(void* o)=0;
  virtual bool enter(void* o)=0;
  virtual bool esc(void* o)=0;
  virtual bool result() const=0;
  virtual Modes mode(void *) const {return Modes::Normal;};
};

/**
* The EmptyCmds is for items that do not handle nav cmds
* they can however react to activation and return a true or false version
*/
template<bool res=false>
struct EmptyCmds:public CmdAgent {
  bool canNav() const override {return false;}
  bool result() const override {return res;};
  bool up (void* o)  override {return false;}
  bool down (void* o)  override {return false;}
  bool enter (void* o)  override {return false;}
  bool esc (void* o)  override {return false;}
};

/**
* The ItemCmd class provides access to navigation functions of a specific item
* system generated this types automatically and maps to object functions
*/
template<typename I,bool res=true>
struct ItemCmd:public CmdAgent {
  bool canNav () const override {return true;}
  bool result() const override {return res;};
  bool up(void* o) override {return ((I*)o)->up();}
  bool down(void* o) override {return ((I*)o)->down();}
  bool enter(void* o) override {return ((I*)o)->enter();}
  bool esc(void* o) override {return ((I*)o)->esc();}
  Modes mode(void* o) const override {return ((I*)o)->mode();};
};

/** @}*/

//////////////////////////////////////////////
template<typename I>
struct Empty:public I {
  template<typename N,typename O,typename H>
  static inline void print(N&,O&,H&) {}
  template<typename N,typename O,typename H>
  static inline void printItem(N&,O&,H&,idx_t) {}
  // constexpr static inline bool isRange() {return false;}
  constexpr static inline idx_t size() {return 0;}
  /// is this item enabled?
  constexpr static inline bool enabled() {return true;}
  /// get enabled status of collection indexed item
  constexpr static inline bool enabled(idx_t) {return true;}
  /// set enabled status of indexed collection member
  static inline void enable(idx_t,bool) {}
  /// returns a dumb agent to be used by navigation
  static inline NavAgent activate();
  /// activate collection item by index
  static inline NavAgent activateItem(idx_t);
  /// the dumb navigation agent, meaning this item does not handle navigation
  static EmptyCmds<false> cmds;
};

/** \defgroup Agents Command and navigation agents
 *  @{
 */

/**
* The NavAgent class allow navigation system access to specific item
* navigation functions.
*/
struct NavAgent {
  void* obj;
  CmdAgent* run;//we will derive this one, it will know the void final type
  inline NavAgent():obj(NULL),run(Empty<>::activate().run) {}
  inline NavAgent(void* o,CmdAgent* r):obj(o),run(r) {}
  inline NavAgent(const NavAgent& o):obj(o.obj),run(o.run) {}
  inline NavAgent operator=(NavAgent&& o) {obj=o.obj;run=o.run;return o;}
  inline operator bool() const {return run->canNav();}
  inline bool canNav() const {return run->canNav();}
  inline bool up() {return run->up(obj);}
  inline bool down() {return run->down(obj);}
  inline bool enter() {return run->enter(obj);}
  inline bool esc() {return run->esc(obj);}
  inline bool result() const {return run->result();};
  inline Modes mode() const {return run->mode(obj);};
};

/**
* @brief means the item has no associated action
* @return always false, the item will not handle navigation
* @details this is a comodity
*/
inline bool doNothing() {return false;}

/**
* The NavHandler class allow menu item to receive navigation commands.
* Navigation functions are mapped automatically
*/
template<typename I>
class NavHandler:public I {
  public:
    using I::I;
    using This=NavHandler<I>;
    inline NavAgent activate() {return {this,&cmds};}
  protected:
    static ItemCmd<This> cmds;
};

/// ActionHanlder, type of action functions to associate with items
using ActionHandler=bool (*)();

/**
* The Action class associates an actikon function with a menu item.
*/
template<typename I,ActionHandler act=doNothing>
class Action:public I {
  public:
    using I::I;
    using This=Action<I,act>;
    inline NavAgent activate() {
      if (act()) return {this,&cmds};
      else return Empty<>::activate();
    }
  protected:
    static EmptyCmds<true> cmds;
};
/** @}*/

template<const char**text,typename I=Empty<>>
struct StaticText:public I {
  template<typename N,typename O,typename H>
  inline void print(N& n,O& o,H& i) {
    o.raw(text[0]);
    I::template print<N,O,H>(n,o,i);
  }
};

template<typename I,typename... II>
class StaticMenu:public StaticMenu<I> {
  public:
    using This=StaticMenu<I>;
    using Next=StaticMenu<II...>;
    friend Next;
    template<typename Prompt>
    static inline idx_t selIdx(Prompt& i,idx_t n) {return n?i.next.selIdx(i,n-1):selIdx(i);}
    constexpr static inline idx_t size() {return Next::size()+1;}
    inline void enable(idx_t n,bool o) {
      if (n) next.enable(n-1,o);
      else I::enable(n,o);
    }
    inline bool enabled(idx_t n) const {
      trace(MDO<<"StaticMenu<I,II...>::enabled"<<endl);
      return n?next.enabled(n-1):I::enabled(0);
    }
    template<typename N,typename O,typename H>
    inline void printItem(N& n,O& o,H& i,idx_t c) {
      // _trace(MDO<<"StaticMenu<...>::printItem "<<c<<endl);
      c>0?i.next.printItem(n,o,next,c-1):I::print(n,o,i);
    }
  protected:
    Next next;
};

template<typename I>
struct StaticMenu<I>:public I {
  using This=StaticMenu<I>;
  constexpr static inline idx_t size() {return 1;}
  inline void enable(idx_t n,bool o) {
    if(!n) I::enable(n,o);
  }
  inline bool enabled(idx_t n) const {
    trace(MDO<<"StaticMenu<I>::enabled"<<endl);
    return n?true:I::enabled(0);
  }
  template<typename N,typename O,typename H>
  inline void print(N& n,O& o,H& i) {/*I::print(n,o,i);*/}
  template<typename N,typename O,typename H>
  static inline void printItem(N& n,O& o,H& i,idx_t c) {
    // _trace(MDO<<"StaticMenu::printItem "<<c<<endl);
    i.I::print(n,o,i);
  }
};


template<typename I,typename N=NavPos<>>
struct ItemNav:public I,public N {
  template<typename O>
  inline void printMenu(O& out) {out.print(*this,out,*this);}
};

template<typename I>
struct StaticItem:public I {
  using This=StaticItem<I>;
  template<typename N,typename O,typename H>
  inline void print(N& n,O& o,H& i) {
    I::template print<N,O,This>(n,o,i);
  }
};

struct Item {
  virtual inline void print(INavNode&,IMenuOut&) {}
};

template<typename I>
struct Prompt:public Item, public I {
  inline void print(INavNode& nav,IMenuOut& out) override {I::print(nav,out,(I&)*this);}
  template<typename N,typename O,typename H>
  inline void print(N& nav,O& out,H& i) {I::print(nav,out,i);}
};

/**
* The AsUnit class signals the format system to handle inner content as an unit (normaly append text after a value)
*/
template<typename I>
struct AsUnit:public I {
  using I::I;
  using This=AsUnit<I>;
  template<typename N,typename O,typename H>
  inline void print(N& nav,O& out,H& i) {
    out.template fmtUnit<true,N,O,This>(nav,out,*this,0);
    I::print(nav,out,i);
    out.template fmtUnit<false,N,O,This>(nav,out,*this,0);
  }
};

/**
* The AsMode class signals the format system to handle inner content as a edit mode cursor.
*/
template<typename I>
struct AsMode:public I {
  using I::I;
  using This=AsMode<I>;
  template<typename N,typename O,typename H>
  inline void print(N& nav,O& out,H& i) {
    out.template fmtMode<true,N,O,This>(nav,out,*this,0);
    I::print(nav,out,i);
    out.template fmtMode<false,N,O,This>(nav,out,*this,0);
  }
};

/**
* The AsValue class signals the format system to handle inner content as a value.
*/
template<typename I>
struct AsValue:public I {
  using I::I;
  using This=AsValue<I>;
  template<typename N,typename O,typename H>
  inline void print(N& nav,O& out,H& i) {
    out.template fmtValue<true,N,O,This>(nav,out,*this,0);
    I::print(nav,out,i);
    out.template fmtValue<false,N,O,This>(nav,out,*this,0);
  }
};
