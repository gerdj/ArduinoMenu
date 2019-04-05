/* -*- C++ -*- */
#pragma once
////////////////////////////////////////////////////
// Rui Azevedo - Apr2019
// neu-rah (ruihfazevedo@gmail.com)
// ouput internal process
// output drivers are a composition between MenuOut<->MenuOutCap
// delivered as O parameter of MenuOutCap template

#include "menu.h"

namespace Menu {
  ///////////////////////////////////////////////////////////////////
  // output interface

  struct MenuOut {
    virtual MenuOut& operator<<(Item&) {return *this;}
    virtual MenuOut& operator<<(const char*) {return *this;}
    virtual MenuOut& operator<<(char) {return *this;}
    virtual MenuOut& operator<<(unsigned char) {return *this;}
    #ifdef ARDUINO
      virtual MenuOut& operator<<(endlObj) {return *this;}
      virtual MenuOut& operator<<(const __FlashStringHelper *i) {return *this;}
    #endif
    virtual void fmtMenu(bool io) {}
    virtual void fmtPanel(bool io) {}
    virtual void fmtTitle(bool io) {}
    virtual void fmtItem(bool io) {}
    virtual void fmtAccel(bool io) {}
    virtual void fmtCursor(bool io) {}
    virtual void fmtLabel(bool io) {}
    virtual void fmtMode(bool io) {}
    virtual void fmtValue(bool io) {}
    virtual void fmtUnit(bool io) {}
    virtual void printMenu() {}
    virtual void setTarget(Item& i) {}
    // virtual Item& getTarget() {return *(Item*)NULL;}
};

  template<typename O> using asMenu=Role<Roles::Menu,O,&MenuOut::fmtMenu>;
  template<typename O> using asPanel=Role<Roles::Panel,O,&MenuOut::fmtPanel>;
  template<typename O> using asTitle=Role<Roles::Title,O,&MenuOut::fmtTitle>;
  template<typename O> using asItem=Role<Roles::Item,O,&MenuOut::fmtItem>;
  template<typename O> using asAccel=Role<Roles::Accel,O,&MenuOut::fmtAccel>;
  template<typename O> using asCursor=Role<Roles::Cursor,O,&MenuOut::fmtCursor>;
  template<typename O> using asLabel=Role<Roles::Label,O,&MenuOut::fmtLabel>;
  template<typename O> using asMode=Role<Roles::Mode,O,&MenuOut::fmtMode>;
  template<typename O> using asValue=Role<Roles::Value,O,&MenuOut::fmtValue>;
  template<typename O> using asUnit=Role<Roles::Unit,O,&MenuOut::fmtUnit>;

  template<typename O>
  struct PrintHead {
    MenuOut& menuOut;
    O& printer;
    size_t pos;
  };

  template<typename O>
  struct MenuOutCap:public MenuOut,public O {
    // using This=MenuOutCap<O>;
    using O::O;
    PrintHead<O> head{*this,*this,0};
    MenuOut& operator<<(Item& i) override;
    MenuOut& operator<<(const char* i) override {O::raw(i);return *this;}
    MenuOut& operator<<(char i) override {O::raw(i);return *this;}
    MenuOut& operator<<(unsigned char i) override {O::raw(i);return *this;}
    #ifdef ARDUINO
      MenuOut& operator<<(endlObj) override {O::raw("\n");return *this;}
      MenuOut& operator<<(const __FlashStringHelper * i) override {O::raw(i);return *this;}
    #endif
    void fmtMenu(bool io) override {O::fmtMenu(head,io);}
    void fmtPanel(bool io) override {O::fmtPanel(head,io);}
    void fmtTitle(bool io) override {O::fmtTitle(head,io);}
    void fmtItem(bool io) override {O::fmtItem(head,io);}
    void fmtAccel(bool io) override {O::fmtAccel(head,io);}
    void fmtCursor(bool io) override {O::fmtCursor(head,io);}
    void fmtLabel(bool io) override {O::fmtLabel(head,io);}
    void fmtMode(bool io) override {O::fmtMode(head,io);}
    void fmtValue(bool io) override {O::fmtValue(head,io);}
    void fmtUnit(bool io) override {O::fmtUnit(head,io);}
    void printMenu() override {
      //TODO: install panel browser here
      O::newView();
      O::printMenuRaw(PrintHead<O>{*this,*this,0},O::getTarget());
    }
    void setTarget(Item& i) override {O::setTarget(i);}
    // Item& getTarget() override {
    //   return O::getTarget();
    // }
  };

  //base for output combinators --------------------------
  struct Void {
    // inline void out(Item& i);
    template<typename T>
    static inline void raw(T) {}//just ignore stuff
    static inline void newView() {}//restart the viewport from the panel definition
    template<typename P> static inline void fmtMenu  (PrintHead<P>,bool io) {}
    template<typename P> static inline void fmtPanel (PrintHead<P>,bool io) {}
    template<typename P> static inline void fmtTitle (PrintHead<P>,bool io) {}
    template<typename P> static inline void fmtItem  (PrintHead<P>,bool io) {}
    template<typename P> static inline void fmtAccel (PrintHead<P>,bool io) {}
    template<typename P> static inline void fmtCursor(PrintHead<P>,bool io) {}
    template<typename P> static inline void fmtLabel (PrintHead<P>,bool io) {}
    template<typename P> static inline void fmtMode  (PrintHead<P>,bool io) {}
    template<typename P> static inline void fmtValue (PrintHead<P>,bool io) {}
    template<typename P> static inline void fmtUnit  (PrintHead<P>,bool io) {}
    enum OUTPUT_BASE {};//do not define this elsewhere
    constexpr static inline bool canNav() {return false;}
    template<typename P> inline void printMenuRaw(PrintHead<P>,const Item&) {}
    template<typename T> using itemFmt=ID<T>;
    template<typename T> using titleFmt=ID<T>;
    template<typename T> using menuFmt=ID<T>;
    template<typename T> using panelFmt=ID<T>;
    // inline void setTarget(Item& i) {}
    // inline Item& getTarget(Item& i) {return *this;}
  };

  //holds scroll position. step should be font size in device coordinates
  template<typename O,int step=1>
  class ScrollPos:public O {
  public:
    //this should be a device only thing
    //not related to menu structure
    //but eventually controlled by it
    inline size_t top() const {return oi;}
    inline size_t scrlUp() {oi+=step;}
    inline size_t scrlDown() {oi-=step;}
    inline size_t scrlTo(size_t i) {oi=i;}
  protected:
    size_t oi;//option index
  };

  //single line scroll controller for text devices
//   template<typename O,int step=1>
//   struct SingleLineScrollCtrl:public ScrollPos<O,step> {
//     using This=ScrollPos<O,step>;
//     inline bool down() {if (O::down()) return This::scrlDown();}
//     inline bool up() {if (O::up()) return This::scrlUp();}
// };

  //bind output to existing device ---------------------------
  //use any stream as menu output
  template<typename Dev, Dev& dev, typename O>
  struct OutDev:public O {
    enum RAW_DEVICE {};
    constexpr bool hasRawDevice() {return true;}
    static inline void raw(const char*i) {dev<<i;}
    static inline void raw(char i) {dev<<i;}
    static inline void raw(unsigned char i) {dev<<i;}
    #ifdef ARDUINO
      static inline void raw(endlObj i) {dev<<i;}
      static inline void raw(const __FlashStringHelper * i) {dev<<i;}
    #endif
    //.. add more type here
  };

};//Menu
