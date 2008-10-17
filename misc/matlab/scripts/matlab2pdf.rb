#!/usr/bin/env ruby

def go(filename)

	latex=<<EOF
\documentclass{article}
\usepackage{amsmath}
\usepackage{xcolor}
\usepackage{listings}

\begin{document}
\lstset{% general command to set parameter(s)
	basicstyle=\footnotesize\ttfamily, % print whole listing small
	keywordstyle=\color{blue}\bfseries,
	% underlined bold black keywords
	identifierstyle=, % nothing happens
	commentstyle=\color{green}, % white comments
	stringstyle=\color{red}\ttfamily, % typewriter type for strings
	tabsize=4,
	language=matlab,
	escapeinside={\%\%}{\^^M},
	%escapeinside={\%\%\%}{M\%\%\%},
	escapebegin=\begin{commentline},
	escapeend=\end{commentline},
	showstringspaces=false} % no special string spaces

\newenvironment{commentline}{\begin{minipage}{\textwidth}\color{violet}\rmfamily}{\end{minipage}}

\lstinputlisting[tabsize=4,language=matlab]{test_gpm.m}
\end{document}
EOF


	temp_dir = ...
	temp_file = Fi
end
