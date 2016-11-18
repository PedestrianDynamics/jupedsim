
NAME=JuPedSim.tex

all: $(NAME)

	xelatex $<
	xelatex $<
	make clean


clean:
	-rm -f *~ *.out *.aux *.dvi *.ps  *.toc *.lol *.lof *.log *.lot *.flg *.bbl *.blg *.idx *.ind *.ilg *.snm *.nav *.fig.bak
