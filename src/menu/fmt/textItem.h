/* -*- C++ -*- */
#pragma once
/**
* @author Rui Azevedo
* @date 10 May 2019
* @copyright 2019 Rui Azevedo
* @brief ArduinoMenu text format, add `\n` at title and item end, print index and text cursor
*/

namespace Menu {
  template<typename O>
  struct TextItemFmt:public O {
    template<bool io,bool toPrint=true>
    inline void fmtItem(Idx n=0,bool s=false,bool e=true,Mode m=Mode::Normal) {
      O::template fmtItem<io,toPrint>(n,s,e,m);
      if (!io) O::nl();
    }
  };
};
