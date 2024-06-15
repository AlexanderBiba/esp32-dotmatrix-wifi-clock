import os

def html_to_cpp_header(html_file, header_file):
  with open(html_file, 'r') as file:
    html_content = file.read()

    cpp_content = f'''
#ifndef APP_WEB_PAGE_H
#define APP_WEB_PAGE_H
const char WebPage[] PROGMEM = R"html({html_content})html";
#endif'''

    with open(header_file, 'w') as file:
      file.write(cpp_content)

if __name__ == "__main__":
  html_file = 'src/webpage.html'
  header_file = 'src/webpage.h'
  html_to_cpp_header(html_file, header_file)
  print(f"Converted {html_file} to {header_file}.")
