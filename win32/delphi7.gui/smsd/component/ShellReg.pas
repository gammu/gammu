{*********************************************************}
{                                                         }
{       Borland Delphi Visual Component Library           }
{                                                         }
{       Copyright (c) 1995, 2001-2002 Borland Corporation }
{                                                         }
{*********************************************************}
unit ShellReg platform;

interface

procedure Register;

implementation

uses Classes, TypInfo, Controls, DesignIntf, ShellCtrls, ShellConsts, RootEdit;

procedure Register;
begin
  GroupDescendentsWith(TShellChangeNotifier, Controls.TControl);
  RegisterComponents(SPalletePage, [TShellTreeView, TShellComboBox, TShellListView,
    TShellChangeNotifier]);
  RegisterPropertyEditor(TypeInfo(TRoot), TShellTreeView, SPropertyName, TRootProperty);
  RegisterPropertyEditor(TypeInfo(TRoot), TShellComboBox, SPropertyName, TRootProperty);
  RegisterPropertyEditor(TypeInfo(TRoot), TShellListView, SPropertyName, TRootProperty);
  RegisterPropertyEditor(TypeInfo(TRoot), TShellChangeNotifier, SPropertyName, TRootProperty);
  RegisterComponentEditor(TShellTreeView, TRootEditor);
  RegisterComponentEditor(TShellListView, TRootEditor);
  RegisterComponentEditor(TShellComboBox, TRootEditor);
  RegisterComponentEditor(TShellChangeNotifier, TRootEditor);
end;

end.
