#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Empty array manipulation (Perl Script)
#
# This script demonstrates the manipulation of empty arrays on properties, 
# qualifiers and named value sets.
#
#***************************************************************************
use strict;
use Win32::OLE;

my $Service;
eval {$Service = Win32::OLE->GetObject("winmgmts:root/default"); };
if (!$@ && defined $Service)
{
	close(STDERR);
	my ($value, $MyClass);

	$MyClass = $Service->Get();
	$MyClass->Path_->{Class} = "EMPTYARRAYTEST00";

	#*************************
	#CASE 1: Property values
	#*************************
	my $Prop = $MyClass->Properties_->Add("p1", 2, 1);
	$Prop->{Value} = [ ];
	$value = $MyClass->Properties_->Item("p1")->{Value}; 
	$value = (defined $value && @$value) ? @$value : -1;
	printf("\nArray upper bound for property value is [-1]: %d\n", $value);
	printf("Base CIM property type is [2]: %d\n ", $Prop->{CIMType});

	#*************************
	#CASE 2: Qualifier values
	#*************************
	$MyClass->Qualifiers_->Add ("q1", []);
	$value = $MyClass->Qualifiers_->Item("q1")->{Value};
	$value = (defined $value && @$value) ? @$value : -1;
	printf("\nArray upper bound for qualifier value is [-1]: %d\n", $value);
	$MyClass->Put_();

	#Now read them back and assign "real values"
	$MyClass = $Service->Get("EMPTYARRAYTEST00");

	$MyClass->Properties_->Item("p1")->{Value} = [12, 34, 56];
	$value = $MyClass->Properties_->Item("p1")->{Value};
	$value = (defined $value && @$value) ? @$value : -1;
	printf("\nArray upper bound for property value is [2]: %d\n", $value-1);
	printf("Base CIM property type is [2]: %d\n ", $Prop->{CIMType});
	
	$MyClass->Properties_->Item("p1")->{Value} = [];
	$value = $MyClass->Properties_->Item("p1")->{Value}; 
	$value = (defined $value && @$value) ? @$value : -1;
	printf("\nArray upper bound for property value is [-1]: %d\n", $value);
	printf("Base CIM property type is [2]: %d\n ", $Prop->{CIMType});

	$MyClass->Qualifiers_->Item("q1")->{Value} = ["Providence", "Melo"];
	$value = $MyClass->Qualifiers_->Item("q1")->{Value};
	$value = (defined $value && @$value) ? @$value : -1;
	printf("\nArray upper bound for qualifier value is [1]: %d\n", $value-1);

	$MyClass->Qualifiers_->Item("q1")->{Value} = [];
	$value = $MyClass->Qualifiers_->Item("q1")->{Value};
	$value = (defined $value && @$value) ? @$value : -1;
	printf("\nArray upper bound for qualifier value is [-1]: %d\n", $value);

	$MyClass->Put_();

	#*************************
	#CASE 3:Named Values
	#*************************
	my $NValueSet = new Win32::OLE 'WbemScripting.SWbemNamedValueSet';
	my $NValue = $NValueSet->Add("Muriel", []);
	$value = $NValue->{Value}; 
	$value = (defined $value && @$value) ? @$value : -1;
	printf("\nArray upper bound for context value is [-1]: %d\n", $value);
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}
