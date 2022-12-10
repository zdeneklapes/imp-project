LOGIN=xlapes02
DOC_NAME=documentation

.PHONY: docs
docs:
	$(MAKE) -C docs && cp docs/out/manual.pdf ./$(DOC_NAME).pdf


.PHONY: clean
clean:
	$(RM) xlapes02.zip $(DOC_NAME).pdf
	$(MAKE) -C docs clean


.PHONY: pack
pack: clean docs
	$(MAKE) -C docs clean
	zip -r $(LOGIN).zip Makefile  \
					  Sources  \
					  Project_Settings  \
					  Includes \
					  Debug  \
					  docs  \
					  $(DOC_NAME).pdf \
					  .cproject  \
					  .project  \
					  .cwGeneratedFileSetLog \
					  .settings