// <copyright file="PSProfiler.cs" company="Microsoft Corporation">
// Copyright (c) 2012 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

using System;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Management.Automation;
using System.Management.Automation.Language;

namespace Microsoft.Windows.PowerShell
{
    /// <summary>
    /// Cmdlet to run profiler on provided scripts.
    /// If Ast switch is set then the Ast object is returned, 
    /// otherwise time elapsed per script line is displayed.
    /// </summary>
    [Cmdlet("Measure", "Script")]
    public sealed class MeasureScriptCommand : PSCmdlet
    {
        [Parameter(Position=0, ValueFromPipeline=true)]
        public string[] Path { get; set; }

        [Parameter]
        public SwitchParameter Ast { get; set; }

        protected override void ProcessRecord()
        {
            foreach (var path in Path)
            {
                ProviderInfo provider;
                var resolvedPath = SessionState.Path.GetResolvedProviderPathFromPSPath(path, out provider).FirstOrDefault();
                if (resolvedPath != null)
                {
                    Profile(resolvedPath);
                }
                else
                {
                    WriteError(new ErrorRecord(new ArgumentException(path), "ProfilerCantResolvePath", ErrorCategory.InvalidArgument, path));
                }
            }
        }

        /// <summary>
        /// Method to read the provided script file and either write
        /// the created instrumented AST object or write elapsed time
        /// for each line in script.
        /// </summary>
        /// <param name="file"></param>
        private void Profile(string file)
        {
            Token[] tokens;
            ParseError[] errors;
            var ast = Parser.ParseFile(file, out tokens, out errors);
            Profiler profiler = new Profiler(ast.Extent);
            var instrumentor = new InstrumentAst { Profiler = profiler };
            ScriptBlockAst newAst = (ScriptBlockAst)ast.Visit(instrumentor);
            if (Ast)
            {
                WriteObject(newAst);
                return;
            }

            ScriptBlock sb = newAst.GetScriptBlock();
            sb.Invoke();

            string[] allLines = File.ReadAllLines(file);
            for (int i = 0; i < allLines.Length; ++i)
            {
                long time = (i >= profiler.StopWatches.Length) ? 0 : profiler.StopWatches[i].ElapsedMilliseconds;
                var result = new PSObject();
                result.Members.Add(new PSNoteProperty("Time", time));
                result.Members.Add(new PSNoteProperty("Line", allLines[i]));
                WriteObject(result);
            }
        }
    }

    /// <summary>
    /// Profiler class manages StopWatch objects for each
    /// script line.
    /// </summary>
    public sealed class Profiler
    {
        internal Stopwatch[] StopWatches { get; set; }

        internal Profiler(IScriptExtent extent)
        {
            int lines = extent.EndLineNumber;
            StopWatches = new Stopwatch[lines];
            for (int i = 0; i < lines; ++i)
            {
                StopWatches[i] = new Stopwatch();
            }
        }

        public void StartLine(int line)
        {
            StopWatches[line].Start();
        }

        public void EndLine(int line)
        {
            StopWatches[line].Stop();
        }
    }

    /// <summary>
    /// Instrumented Ast class.  Builds an Ast that makes callbacks
    /// into Profiler for measuring statement execution times.
    /// This implementation is needed to add StartLine and StopLine 
    /// profiler object call backs that will record the execution 
    /// time for each script statement.  A complete ICustomAstVistor
    /// implementation must be created and so all methods except for
    /// "VisitStatements" are boiler plate implementations for the
    /// ICustomAstVistor interface.
    /// </summary>
    public sealed class InstrumentAst : ICustomAstVisitor
    {
        internal Profiler Profiler { get; set; }

        public object VisitScriptBlock(ScriptBlockAst scriptBlockAst)
        {
            var newParamBlock = VisitElement(scriptBlockAst.ParamBlock);
            var newBeginBlock = VisitElement(scriptBlockAst.BeginBlock);
            var newProcessBlock = VisitElement(scriptBlockAst.ProcessBlock);
            var newEndBlock = VisitElement(scriptBlockAst.EndBlock);
            var newDynamicParamBlock = VisitElement(scriptBlockAst.DynamicParamBlock);
            return new ScriptBlockAst(scriptBlockAst.Extent, newParamBlock, newBeginBlock, newProcessBlock, newEndBlock,
                                      newDynamicParamBlock);
        }

        public T[] VisitElements<T>(ReadOnlyCollection<T> elements) where T : Ast
        {
            if (elements == null)
            {
                return new T[0];
            }

            var newElements = new List<T>();

            foreach (T t in elements)
            {
                newElements.Add((T)t.Visit(this));
            }
            return newElements.ToArray();
        }

        public T VisitElement<T>(T element) where T : Ast
        {
            if (element == null)
                return null;
            return (T)element.Visit(this);
        }

        /// <summary>
        /// Inserts StartLine and EndLine Profiler callbacks for each
        /// script statement.
        /// </summary>
        /// <param name="statements"></param>
        /// <returns></returns>
        public StatementAst[] VisitStatements(ReadOnlyCollection<StatementAst> statements)
        {
            var newStatements = new List<StatementAst>();
            foreach (var statement in statements)
            {
                bool instrument = (statement is PipelineBaseAst);
                IScriptExtent extent = statement.Extent;
                if (instrument)
                {
                    var startLine = new CommandExpressionAst(
                        extent, new InvokeMemberExpressionAst(extent,
                                                              new ConstantExpressionAst(extent, Profiler),
                                                              new StringConstantExpressionAst(extent, "StartLine", StringConstantType.BareWord),
                                                              new [] { new ConstantExpressionAst(extent, extent.StartLineNumber - 1) }, false), null);
                    var pipe = new PipelineAst(extent, startLine);
                    newStatements.Add(pipe);
                }
                newStatements.Add(VisitElement(statement));
                if (instrument)
                {
                    var endLine = new CommandExpressionAst(
                        extent, new InvokeMemberExpressionAst(extent,
                                                              new ConstantExpressionAst(extent, Profiler),
                                                              new StringConstantExpressionAst(extent, "EndLine", StringConstantType.BareWord),
                                                              new[] { new ConstantExpressionAst(extent, extent.StartLineNumber - 1) }, false), null);
                    var pipe = new PipelineAst(extent, endLine);
                    newStatements.Add(pipe);                    
                }
            }
            return newStatements.ToArray();
        }

        public object VisitNamedBlock(NamedBlockAst namedBlockAst)
        {
            var newTraps = VisitElements(namedBlockAst.Traps);
            var newStatements = VisitStatements(namedBlockAst.Statements);
            var statementBlock = new StatementBlockAst(namedBlockAst.Extent, newStatements, newTraps);
            return new NamedBlockAst(namedBlockAst.Extent, namedBlockAst.BlockKind, statementBlock, namedBlockAst.Unnamed);
        }

        public object VisitFunctionDefinition(FunctionDefinitionAst functionDefinitionAst)
        {
            var newBody = VisitElement(functionDefinitionAst.Body);
            return new FunctionDefinitionAst(functionDefinitionAst.Extent, functionDefinitionAst.IsFilter,
                                             functionDefinitionAst.IsWorkflow, functionDefinitionAst.Name,
                                             VisitElements(functionDefinitionAst.Parameters), newBody);
        }

        public object VisitStatementBlock(StatementBlockAst statementBlockAst)
        {
            var newStatements = VisitStatements(statementBlockAst.Statements);
            var newTraps = VisitElements(statementBlockAst.Traps);
            return new StatementBlockAst(statementBlockAst.Extent, newStatements, newTraps);
        }

        public object VisitIfStatement(IfStatementAst ifStmtAst)
        {
            var newClauses = (from clause in ifStmtAst.Clauses
                              let newClauseTest = VisitElement(clause.Item1)
                              let newStatementBlock = VisitElement(clause.Item2)
                              select new Tuple<PipelineBaseAst, StatementBlockAst>(newClauseTest, newStatementBlock));
            var newElseClause = VisitElement(ifStmtAst.ElseClause);
            return new IfStatementAst(ifStmtAst.Extent, newClauses, newElseClause);
        }

        public object VisitTrap(TrapStatementAst trapStatementAst)
        {
            return new TrapStatementAst(trapStatementAst.Extent, VisitElement(trapStatementAst.TrapType), VisitElement(trapStatementAst.Body));
        }

        public object VisitSwitchStatement(SwitchStatementAst switchStatementAst)
        {
            var newCondition = VisitElement(switchStatementAst.Condition);
            var newClauses = (from clause in switchStatementAst.Clauses
                              let newClauseTest = VisitElement(clause.Item1)
                              let newStatementBlock = VisitElement(clause.Item2)
                              select new Tuple<ExpressionAst, StatementBlockAst>(newClauseTest, newStatementBlock));
            var newDefault = VisitElement(switchStatementAst.Default);
            return new SwitchStatementAst(switchStatementAst.Extent, switchStatementAst.Label,
                                          newCondition,
                                          switchStatementAst.Flags, newClauses, newDefault);
        }

        public object VisitDataStatement(DataStatementAst dataStatementAst)
        {
            var newBody = VisitElement(dataStatementAst.Body);
            var newCommandsAllowed = VisitElements(dataStatementAst.CommandsAllowed);
            return new DataStatementAst(dataStatementAst.Extent, dataStatementAst.Variable, newCommandsAllowed, newBody);
        }

        public object VisitForEachStatement(ForEachStatementAst forEachStatementAst)
        {
            var newVariable = VisitElement(forEachStatementAst.Variable);
            var newCondition = VisitElement(forEachStatementAst.Condition);
            var newBody = VisitElement(forEachStatementAst.Body);
            return new ForEachStatementAst(forEachStatementAst.Extent, forEachStatementAst.Label, ForEachFlags.None,
                                           newVariable, newCondition, newBody);
        }

        public object VisitDoWhileStatement(DoWhileStatementAst doWhileStatementAst)
        {
            var newCondition = VisitElement(doWhileStatementAst.Condition);
            var newBody = VisitElement(doWhileStatementAst.Body);
            return new DoWhileStatementAst(doWhileStatementAst.Extent, doWhileStatementAst.Label, newCondition, newBody);
        }

        public object VisitForStatement(ForStatementAst forStatementAst)
        {
            var newInitializer = VisitElement(forStatementAst.Initializer);
            var newCondition = VisitElement(forStatementAst.Condition);
            var newIterator = VisitElement(forStatementAst.Iterator);
            var newBody = VisitElement(forStatementAst.Body);
            return new ForStatementAst(forStatementAst.Extent, forStatementAst.Label, newInitializer,
                                       newCondition, newIterator, newBody);
        }

        public object VisitWhileStatement(WhileStatementAst whileStatementAst)
        {
            var newCondition = VisitElement(whileStatementAst.Condition);
            var newBody = VisitElement(whileStatementAst.Body);
            return new WhileStatementAst(whileStatementAst.Extent, whileStatementAst.Label, newCondition, newBody);
        }

        public object VisitCatchClause(CatchClauseAst catchClauseAst)
        {
            var newBody = VisitElement(catchClauseAst.Body);
            return new CatchClauseAst(catchClauseAst.Extent, catchClauseAst.CatchTypes, newBody);
        }

        public object VisitTryStatement(TryStatementAst tryStatementAst)
        {
            var newBody = VisitElement(tryStatementAst.Body);
            var newCatchClauses = VisitElements(tryStatementAst.CatchClauses);
            var newFinally = VisitElement(tryStatementAst.Finally);
            return new TryStatementAst(tryStatementAst.Extent, newBody, newCatchClauses, newFinally);
        }

        public object VisitDoUntilStatement(DoUntilStatementAst doUntilStatementAst)
        {
            var newCondition = VisitElement(doUntilStatementAst.Condition);
            var newBody = VisitElement(doUntilStatementAst.Body);
            return new DoUntilStatementAst(doUntilStatementAst.Extent, doUntilStatementAst.Label,
                                           newCondition, newBody);
        }

        public object VisitParamBlock(ParamBlockAst paramBlockAst)
        {
            var newAttributes = VisitElements(paramBlockAst.Attributes);
            var newParameters = VisitElements(paramBlockAst.Parameters);
            return new ParamBlockAst(paramBlockAst.Extent, newAttributes, newParameters);
        }

        public object VisitErrorStatement(ErrorStatementAst errorStatementAst)
        {
            return errorStatementAst;
        }

        public object VisitErrorExpression(ErrorExpressionAst errorExpressionAst)
        {
            return errorExpressionAst;
        }

        public object VisitTypeConstraint(TypeConstraintAst typeConstraintAst)
        {
            return new TypeConstraintAst(typeConstraintAst.Extent, typeConstraintAst.TypeName);
        }

        public object VisitAttribute(AttributeAst attributeAst)
        {
            var newPositionalArguments = VisitElements(attributeAst.PositionalArguments);
            var newNamedArguments = VisitElements(attributeAst.NamedArguments);
            return new AttributeAst(attributeAst.Extent, attributeAst.TypeName, newPositionalArguments, newNamedArguments);
        }

        public object VisitNamedAttributeArgument(NamedAttributeArgumentAst namedAttributeArgumentAst)
        {
            var newArgument = VisitElement(namedAttributeArgumentAst.Argument);
            return new NamedAttributeArgumentAst(namedAttributeArgumentAst.Extent,
                                                 namedAttributeArgumentAst.ArgumentName, newArgument,
                                                 namedAttributeArgumentAst.ExpressionOmitted);
        }

        public object VisitParameter(ParameterAst parameterAst)
        {
            var newName = VisitElement(parameterAst.Name);
            var newAttributes = VisitElements(parameterAst.Attributes);
            var newDefaultValue = VisitElement(parameterAst.DefaultValue);
            return new ParameterAst(parameterAst.Extent, newName, newAttributes, newDefaultValue);
        }

        public object VisitBreakStatement(BreakStatementAst breakStatementAst)
        {
            var newLabel = VisitElement(breakStatementAst.Label);
            return new BreakStatementAst(breakStatementAst.Extent, newLabel);
        }

        public object VisitContinueStatement(ContinueStatementAst continueStatementAst)
        {
            var newLabel = VisitElement(continueStatementAst.Label);
            return new ContinueStatementAst(continueStatementAst.Extent, newLabel);
        }

        public object VisitReturnStatement(ReturnStatementAst returnStatementAst)
        {
            var newPipeline = VisitElement(returnStatementAst.Pipeline);
            return new ReturnStatementAst(returnStatementAst.Extent, newPipeline);
        }

        public object VisitExitStatement(ExitStatementAst exitStatementAst)
        {
            var newPipeline = VisitElement(exitStatementAst.Pipeline);
            return new ExitStatementAst(exitStatementAst.Extent, newPipeline);
        }

        public object VisitThrowStatement(ThrowStatementAst throwStatementAst)
        {
            var newPipeline = VisitElement(throwStatementAst.Pipeline);
            return new ThrowStatementAst(throwStatementAst.Extent, newPipeline);
        }

        public object VisitAssignmentStatement(AssignmentStatementAst assignmentStatementAst)
        {
            var newLeft = VisitElement(assignmentStatementAst.Left);
            var newRight = VisitElement(assignmentStatementAst.Right);
            return new AssignmentStatementAst(assignmentStatementAst.Extent, newLeft, assignmentStatementAst.Operator,
                                              newRight, assignmentStatementAst.ErrorPosition);
        }

        public object VisitPipeline(PipelineAst pipelineAst)
        {
            var newPipeElements = VisitElements(pipelineAst.PipelineElements);
            return new PipelineAst(pipelineAst.Extent, newPipeElements);
        }

        public object VisitCommand(CommandAst commandAst)
        {
            var newCommandElements = VisitElements(commandAst.CommandElements);
            var newRedirections = VisitElements(commandAst.Redirections);
            return new CommandAst(commandAst.Extent, newCommandElements, commandAst.InvocationOperator, newRedirections);
        }

        public object VisitCommandExpression(CommandExpressionAst commandExpressionAst)
        {
            var newExpression = VisitElement(commandExpressionAst.Expression);
            var newRedirections = VisitElements(commandExpressionAst.Redirections);
            return new CommandExpressionAst(commandExpressionAst.Extent, newExpression, newRedirections);
        }

        public object VisitCommandParameter(CommandParameterAst commandParameterAst)
        {
            var newArgument = VisitElement(commandParameterAst.Argument);
            return new CommandParameterAst(commandParameterAst.Extent, commandParameterAst.ParameterName, newArgument,
                                           commandParameterAst.ErrorPosition);
        }

        public object VisitFileRedirection(FileRedirectionAst fileRedirectionAst)
        {
            var newFile = VisitElement(fileRedirectionAst.Location);
            return new FileRedirectionAst(fileRedirectionAst.Extent, fileRedirectionAst.FromStream, newFile,
                                          fileRedirectionAst.Append);
        }

        public object VisitMergingRedirection(MergingRedirectionAst mergingRedirectionAst)
        {
            return new MergingRedirectionAst(mergingRedirectionAst.Extent, mergingRedirectionAst.FromStream,
                                             mergingRedirectionAst.ToStream);
        }

        public object VisitBinaryExpression(BinaryExpressionAst binaryExpressionAst)
        {
            var newLeft = VisitElement(binaryExpressionAst.Left);
            var newRight = VisitElement(binaryExpressionAst.Right);
            return new BinaryExpressionAst(binaryExpressionAst.Extent, newLeft, binaryExpressionAst.Operator, newRight,
                                           binaryExpressionAst.ErrorPosition);
        }

        public object VisitUnaryExpression(UnaryExpressionAst unaryExpressionAst)
        {
            var newChild = VisitElement(unaryExpressionAst.Child);
            return new UnaryExpressionAst(unaryExpressionAst.Extent, unaryExpressionAst.TokenKind, newChild);
        }

        public object VisitConvertExpression(ConvertExpressionAst convertExpressionAst)
        {
            var newChild = VisitElement(convertExpressionAst.Child);
            var newTypeConstraint = VisitElement(convertExpressionAst.Type);
            return new ConvertExpressionAst(convertExpressionAst.Extent, newTypeConstraint, newChild);
        }

        public object VisitTypeExpression(TypeExpressionAst typeExpressionAst)
        {
            return new TypeExpressionAst(typeExpressionAst.Extent, typeExpressionAst.TypeName);
        }

        public object VisitConstantExpression(ConstantExpressionAst constantExpressionAst)
        {
            return new ConstantExpressionAst(constantExpressionAst.Extent, constantExpressionAst.Value);
        }

        public object VisitStringConstantExpression(StringConstantExpressionAst stringConstantExpressionAst)
        {
            return new StringConstantExpressionAst(stringConstantExpressionAst.Extent, stringConstantExpressionAst.Value,
                                                   stringConstantExpressionAst.StringConstantType);
        }

        public object VisitSubExpression(SubExpressionAst subExpressionAst)
        {
            var newStatementBlock = VisitElement(subExpressionAst.SubExpression);
            return new SubExpressionAst(subExpressionAst.Extent, newStatementBlock);
        }

        public object VisitUsingExpression(UsingExpressionAst usingExpressionAst)
        {
            var newUsingExpr = VisitElement(usingExpressionAst.SubExpression);
            return new UsingExpressionAst(usingExpressionAst.Extent, newUsingExpr);
        }

        public object VisitVariableExpression(VariableExpressionAst variableExpressionAst)
        {
            return new VariableExpressionAst(variableExpressionAst.Extent, variableExpressionAst.VariablePath.UserPath,
                                             variableExpressionAst.Splatted);
        }

        public object VisitMemberExpression(MemberExpressionAst memberExpressionAst)
        {
            var newExpr = VisitElement(memberExpressionAst.Expression);
            var newMember = VisitElement(memberExpressionAst.Member);
            return new MemberExpressionAst(memberExpressionAst.Extent, newExpr, newMember, memberExpressionAst.Static);
        }

        public object VisitInvokeMemberExpression(InvokeMemberExpressionAst invokeMemberExpressionAst)
        {
            var newExpression = VisitElement(invokeMemberExpressionAst.Expression);
            var newMethod = VisitElement(invokeMemberExpressionAst.Member);
            var newArguments = VisitElements(invokeMemberExpressionAst.Arguments);
            return new InvokeMemberExpressionAst(invokeMemberExpressionAst.Extent, newExpression, newMethod,
                                                 newArguments, invokeMemberExpressionAst.Static);
        }

        public object VisitArrayExpression(ArrayExpressionAst arrayExpressionAst)
        {
            var newStatementBlock = VisitElement(arrayExpressionAst.SubExpression);
            return new ArrayExpressionAst(arrayExpressionAst.Extent, newStatementBlock);
        }

        public object VisitArrayLiteral(ArrayLiteralAst arrayLiteralAst)
        {
            var newArrayElements = VisitElements(arrayLiteralAst.Elements);
            return new ArrayLiteralAst(arrayLiteralAst.Extent, newArrayElements);
        }

        public object VisitHashtable(HashtableAst hashtableAst)
        {
            var newKeyValuePairs = new List<Tuple<ExpressionAst, StatementAst>>();
            foreach (var keyValuePair in hashtableAst.KeyValuePairs)
            {
                var newKey = VisitElement(keyValuePair.Item1);
                var newValue = VisitElement(keyValuePair.Item2);
                newKeyValuePairs.Add(Tuple.Create(newKey, newValue));
            }
            return new HashtableAst(hashtableAst.Extent, newKeyValuePairs);
        }

        public object VisitScriptBlockExpression(ScriptBlockExpressionAst scriptBlockExpressionAst)
        {
            var newScriptBlock = VisitElement(scriptBlockExpressionAst.ScriptBlock);
            return new ScriptBlockExpressionAst(scriptBlockExpressionAst.Extent, newScriptBlock);
        }

        public object VisitParenExpression(ParenExpressionAst parenExpressionAst)
        {
            var newPipeline = VisitElement(parenExpressionAst.Pipeline);
            return new ParenExpressionAst(parenExpressionAst.Extent, newPipeline);
        }

        public object VisitExpandableStringExpression(ExpandableStringExpressionAst expandableStringExpressionAst)
        {
            return new ExpandableStringExpressionAst(expandableStringExpressionAst.Extent,
                                                     expandableStringExpressionAst.Value,
                                                     expandableStringExpressionAst.StringConstantType);
        }

        public object VisitIndexExpression(IndexExpressionAst indexExpressionAst)
        {
            var newTargetExpression = VisitElement(indexExpressionAst.Target);
            var newIndexExpression = VisitElement(indexExpressionAst.Index);
            return new IndexExpressionAst(indexExpressionAst.Extent, newTargetExpression, newIndexExpression);
        }

        public object VisitAttributedExpression(AttributedExpressionAst attributedExpressionAst)
        {
            var newAttribute = VisitElement(attributedExpressionAst.Attribute);
            var newChild = VisitElement(attributedExpressionAst.Child);
            return new AttributedExpressionAst(attributedExpressionAst.Extent, newAttribute, newChild);
        }

        public object VisitBlockStatement(BlockStatementAst blockStatementAst)
        {
            var newBody = VisitElement(blockStatementAst.Body);
            return new BlockStatementAst(blockStatementAst.Extent, blockStatementAst.Kind, newBody);
        }
    }
}
