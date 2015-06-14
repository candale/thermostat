fis=letter
latex -interaction=nonstopmode $fis.tex
dvips -o $fis.ps $fis.dvi
ps2pdf $fis.ps
