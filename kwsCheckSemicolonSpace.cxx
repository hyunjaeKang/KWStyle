/*=========================================================================

  Program:   KWStyle - Kitware Style Checker
  Module:    kwsCheckSemicolonSpace.cxx

  Copyright (c) Kitware, Inc.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsParser.h"

namespace kws {


/** Check the number of space between the end of the declaration
 *  and the semicolon */
bool Parser::CheckSemicolonSpace(unsigned long max)
{
  m_TestsDone[SEMICOLON_SPACE] = true;
  char* val = new char[255];
  sprintf(val,"Semicolons = %ld max spaces",max);
  m_TestsDescription[SEMICOLON_SPACE] = val;
  delete [] val;

  bool hasError = false;
  long int posSemicolon = static_cast<long int>(m_BufferNoComment.find(";",0));
  while(posSemicolon != -1)
    {
    // We try to find the word before that
    long i=posSemicolon-1;
    unsigned long space = 0;
    while(i>=0)
      {
       if(m_BufferNoComment[i] == ' ')
        {
        if(!IsBetweenQuote(i))
          {
          space++;
          }
        }
       else
        {
        if(m_BufferNoComment[i] != '\n' && space > max)
          {
          Error error;
          error.line = this->GetLineNumber(posSemicolon,true);
          error.line2 = error.line;
          error.number = SEMICOLON_SPACE;
          error.description = "Number of spaces before semicolon exceed: ";
          char* localval = new char[10];
          sprintf(localval,"%ld",space);
          error.description += localval;
          error.description += " (max=";
          delete [] localval;
          localval = new char[10];
          sprintf(localval,"%ld",max);
          error.description += localval;
          error.description += ")";
          delete [] localval;
          m_ErrorList.push_back(error);
          hasError = true;

          if(m_FixFile)
            {
            this->ReplaceCharInFixedBuffer(this->GetPositionWithComments(posSemicolon-space),space,"");
            }
          } // space > max
        break;
        }
      i--;
      }
    posSemicolon = static_cast<long int>(m_BufferNoComment.find(";",posSemicolon+1));
    }

  // We also check if we have two or more semicolons following each other
  posSemicolon = static_cast<long int>(m_BufferNoComment.find(";",0));
  while(posSemicolon != -1)
    {
    // We look forward
    unsigned long i=posSemicolon+1;
    while(i<m_BufferNoComment.size())
      {
      if(m_BufferNoComment[i] == ' ' || m_BufferNoComment[i] == '\r' || m_BufferNoComment[i] == '\n')
        {
        // this is normal
        }
      else if(m_BufferNoComment[i] == ';')
        {
        // if we have for ( ;; ) we don't report
        // or ";;"
        if(!this->IsBetweenCharsFast('(',')',i)
           && !this->IsBetweenQuote(i))
          {
          Error error;
          error.line = this->GetLineNumber(i,true);
          error.line2 = error.line;
          error.number = SEMICOLON_SPACE;
          error.description = "Too many semicolons";
          m_ErrorList.push_back(error);
          hasError = true;

          if(m_FixFile)
            {
            this->ReplaceCharInFixedBuffer(this->GetPositionWithComments(i),1,"");
            }
          }
        }
      else
        {
        break;
        }
      i++;
      }
    posSemicolon = static_cast<long int>(m_BufferNoComment.find(";",i+1));
    }


  // We also check if we have unecessary semicolons
  posSemicolon = static_cast<long int>(m_BufferNoComment.find(";",0));
  while(posSemicolon != -1)
    {
    // check if we have a } before
    long int pos = posSemicolon;
    pos--;
    while(pos>0 &&
      (m_BufferNoComment[pos] == ' '
      || m_BufferNoComment[pos] == '\n'
      || m_BufferNoComment[pos] == '\r'))
      {
      pos--;
      }

    if(m_BufferNoComment[pos] == '}')
      {
      bool error = true;
      // We check that this is not a class
      long int openingChar = static_cast<long int>(this->FindOpeningChar('}','{',pos,true));

      long int classPos = static_cast<long int>(m_BufferNoComment.find("class"));
      while(classPos != -1)
        {
        if(classPos != -1 && openingChar!= -1)
          {
          for(unsigned long i=classPos;i<(unsigned long)openingChar+1;i++)
            {
            if(m_BufferNoComment[i] == '{')
              {
              classPos = i;
              break;
              }
            }
          if(openingChar == classPos)
            {
            error = false;
            }
          }
        classPos = static_cast<long int>(m_BufferNoComment.find("class",classPos+1));
        }

      // We check that this is not a enum
      long int enumPos = static_cast<long int>(m_BufferNoComment.find("enum"));
      while(enumPos != -1)
        {
        if(enumPos != -1 && openingChar!= -1)
          {
          for(unsigned long i=enumPos;i<(unsigned long)openingChar+1;i++)
            {
            if(m_BufferNoComment[i] == '{')
              {
              enumPos = i;
              break;
              }
            }
          if(openingChar == enumPos)
            {
            error = false;
            }
          }
        enumPos = static_cast<long int>(m_BufferNoComment.find("enum",enumPos+1));
        }

      std::string word = this->FindPreviousWord(openingChar);
      if(word == "enum")
        {
        error = false;
        }

      // We check that this is not a struct
      long int structPos = static_cast<long int>(m_BufferNoComment.find("struct"));
      while(structPos != -1)
        {
        if(structPos != -1 && openingChar!= -1)
          {
          for(unsigned long i=structPos;i<(unsigned long)openingChar+1;i++)
            {
            if(m_BufferNoComment[i] == '{')
              {
              structPos = i;
              break;
              }
            }
          if(openingChar == structPos)
            {
            error = false;
            }
          }
        structPos = static_cast<long int>(m_BufferNoComment.find("struct",structPos+1));
        }

      word = this->FindPreviousWord(openingChar);
      if(word == "struct")
        {
        error = false;
        }

      // We check that this is not a union
      long int unionPos = static_cast<long int>(m_BufferNoComment.find("union"));
      while(unionPos != -1)
        {
        if(unionPos != -1 && openingChar!= -1)
          {
          for(unsigned long i=unionPos;i<(unsigned long)openingChar+1;i++)
            {
            if(m_BufferNoComment[i] == '{')
              {
              unionPos = i;
              break;
              }
            }
          if(openingChar == unionPos)
            {
            error = false;
            }
          }
        unionPos = static_cast<long int>(m_BufferNoComment.find("union",unionPos+1));
        }

      word = this->FindPreviousWord(openingChar);
      if(word == "union")
        {
        error = false;
        }

      // We check that this is not a static variable
      long int i = pos--;
      while(i>0)
        {
        if(m_BufferNoComment[i]==';'
          || (m_BufferNoComment[i]==')'))
          {
          break;
          }
        i--;
        }
      std::string substring = m_BufferNoComment.substr(i,openingChar-i);
      if(substring.find("static") != std::string::npos)
        {
        error = false;
        }

      // We check that this is not a variable definition
      i = posSemicolon-1;
      while(i>0)
        {
        if(m_BufferNoComment[i]=='}')
          {
          long int localopeningChar = static_cast<long int>(this->FindOpeningChar('}','{',i,true));
          long int j = localopeningChar-1;
          while(j>0 && (m_BufferNoComment[j]==' '
            || m_BufferNoComment[j]=='\n'
            || m_BufferNoComment[j]=='\r')
            )
            {
            j--;
            }

          if(m_BufferNoComment[j]=='=')
            {
            error = false;
            }
          break;
          }
        i--;
        }

      // Check if the {} is empty or not
      if(openingChar!= -1)
        {
        bool empty = true;
        for(unsigned long index=openingChar+1;index<(unsigned long)pos;index++)
          {
          if(m_BufferNoComment[index] != ' ' || m_BufferNoComment[index] != '\r'
            || m_BufferNoComment[index] != '\n')
            {
            empty = false;
            break;
            }
          }
        if(empty)
          {
          error = false;
          }
        }

      if(error)
        {
        Error localerror;
        localerror.line = this->GetLineNumber(posSemicolon,true);
        localerror.line2 = localerror.line;
        localerror.number = SEMICOLON_SPACE;
        localerror.description = "Unnecessary semicolon";
        m_ErrorList.push_back(localerror);
        hasError = true;
        }
      }
    posSemicolon = static_cast<long int>(m_BufferNoComment.find(";",posSemicolon+1));
    }

  return !hasError;
}

} // end namespace kws
