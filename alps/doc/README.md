[//]: # ( (c) Copyright 2016 Hewlett Packard Enterprise Development LP             )
[//]: # (                                                                          )
[//]: # ( Licensed under the Apache License, Version 2.0 (the "License");          )
[//]: # ( you may not use this file except in compliance with the License.         )
[//]: # ( You may obtain a copy of the License at                                  )
[//]: # (                                                                          )
[//]: # (     http://www.apache.org/licenses/LICENSE-2.0                           )
[//]: # (                                                                          )
[//]: # ( Unless required by applicable law or agreed to in writing, software      )
[//]: # ( distributed under the License is distributed on an "AS IS" BASIS,        )
[//]: # ( WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. )
[//]: # ( See the License for the specific language governing permissions and      )
[//]: # ( limitations under the License.                                           )


Welcome to the Alps documentation!

This readme will walk you through navigating and building the Alps
documentation, which is included here with the Alps source code. 

Read on to learn more about viewing documentation in plain text (i.e., markdown) 
or building the documentation yourself. Why build it yourself? So that you have 
the docs that corresponds to whichever version of Alps you currently have 
checked out of revision control.

## Generating the Documentation

We include the Alps documentation as part of the source (as opposed to using 
a hosted wiki, such as the github wiki, as the definitive documentation) to 
enable the documentation to evolve along with the source code and be captured 
by revision control (currently git). This way the code automatically includes 
the version of the documentation that is relevant regardless of which version 
or release you have checked out or downloaded.

To compile the documentation:

    $ cd $ALPINISM/doc
    $ doxygen
 
Generated documentation is placed under: 
 - HTML: $ALPS/doc/dox/html
 - LaTeX: $ALPS/doc/dox/latex

## Adding the Alps logo to the Latex Documentation

- Copy $ALPS/doc/figures/alps-logo.pdf to $ALPS/doc/dox/latex
- Add the following to the titlepage in refman.tex 

\vspace{1cm}
\includegraphics{alps-logo}
