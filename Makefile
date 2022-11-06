LOGIN=xlapes02
DOC_NAME=documentation

.PHONY: docs
docs:
	$(MAKE) -C docs && cp docs/manual.pdf ./$(DOC_NAME).pdf


.PHONY: clean
clean:
	$(RM) xlapes02.zip
	$(MAKE) -C docs clean


.PHONY: pack
pack: docs clean
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