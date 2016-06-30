VERSION := 1.0
all:
	@mvn clean compile assembly:single
	@mv target/LoWPANSniffer-$(VERSION)-jar-with-dependencies.jar ./LoWPANSniffer.jar
	@$(RM) -rf target
