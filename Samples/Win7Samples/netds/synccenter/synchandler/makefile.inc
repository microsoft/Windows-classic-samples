!ifndef NOSELFREG
!ifndef SELFREGNAME
SELFREGNAME = selfreg.inf
!endif
!ifndef SELFREGSOURCE
SELFREGSOURCE = ..\selfreg.inx
!endif

$(SELFREGNAME) : $(SELFREGSOURCE)
        $(C_PREPROCESSOR_NAME) $(C_PREPROCESSOR_FLAGS:-DNT =) $** > $*.cln
        $(CLEANINF) $*.cln $@
        del $*.cln

Resources.rc : $(SELFREGNAME)
!endif
