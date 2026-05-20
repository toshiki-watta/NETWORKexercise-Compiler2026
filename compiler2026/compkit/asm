#!/usr/bin/env perl

# Assembler for P1
# Copyright (C) 1996 Toyohashi University of Technology

# global variables

($procname) = ($0 =~ m#([^/]+)$#);

$magic_number = 'PMV1.0';

%nmemid = ();		# nmemonic -> id
%opid = ();		# opcode -> id
@nmemonic = ();		# id -> nmemonic
@optype = ();		# id -> optype
@opcode = ();		# id -> opcode
@execycle = ();		# id -> execution cycle
@regname = ('R0','R1','R2','R3','SP','BR', '??', '??');
%regno = ('r0', '0', 'R0', '0',
	  'r1', '1', 'R1', '1',
	  'r2', '2', 'R2', '2',
	  'r3', '3', 'R3', '3',
	  'r4', '4', 'R4', '4',
	  'sp', '4', 'SP', '4',
	  'r5', '5', 'R5', '5',
	  'br', '5', 'BR', '5' );

&init_op_definition;

@progseg = ();

$asm_usage = <<'@@@';
USAGE: asm [<options>] [<input-file> [<output-file>]]
       <options> are:
           -h   show this message (and exit immediately)
           -d   disassemble
           -r   assemble & run
                ( discard object-code if <output-file> is not given )
           -t   trace mode
           -s   single-step execution mode
           --   specify the end of options

       NOTE:
          reads from stdin if <input-file> is not given or given as `-'.
          writes to stdout if <output-file> is not given or given as `-'.
@@@

# main procedure

%OPTIONS = ();

MAIN: {
  &getopt("hrdts");
  &fatal($asm_usage) if (scalar(@ARGV) > 2 || defined($OPTIONS{'h'}));
  local($exec_mode) = $OPTIONS{'r'} || $OPTIONS{'t'} || $OPTIONS{'s'};
  &init_asm;
  if (defined($OPTIONS{'d'})) {
    &read_code($ARGV[0]);
    &init_disasm;
    &disasm_code($ARGV[1]);
  } else {
    &encode_asm($ARGV[0]);
    &write_code($ARGV[1]) if (! $exec_mode || defined($ARGV[1]));
  }
  if ($exec_mode) {
    &init_sr;
    &exec_code;
  }
}
exit(0);

# subroutines

sub getopt {
  local(@opts) = split(//, $_[0]);
  local($arg, $opt);
  %OPTIONS = ();
  while (scalar(@ARGV) > 0 && $ARGV[0] =~ /^-./) {
    $arg = shift(@ARGV);
    return if ($arg eq '--');
    $arg = substr($arg, 1);
    foreach (@opts) {
      $OPTIONS{$_} = 1 if (index($arg, $_) >= $[);
    }
  }
}

sub init_asm {
  package asm;

  %labels = ();
  $lineno = 0;

  sub main'encode_asm {
    local($srcfile) = @_;
    $srcfile = '-' unless (defined($srcfile));
    open(SRC, "<$srcfile")
      || &main'fatal("can't open source file `$srcfile'\n");
    # pass 1
    do {
      local($first, $id);
      @progseg = ();
      $lineno = 0;
      line: while (<SRC>) {
	$lineno++;
	$_ = &skipblanks($_);
        label: {
	  next line if ($_ eq '');
	  ($first, $_) = &getident($_);
	  $_ = &skipblanks($_);
	  if (/^:/) {
	    $_ = &skipblanks($');
	    &asmfatal("label redefined") if (defined($labels{$first}));
	    $labels{$first} = &half(scalar(@progseg));
	    redo label;
	  }
	}
	$id = $main'nmemid{$first};
	&asmfatal("illegal nmemonic") unless (defined($id));
	push(@progseg, &encode($id, $_));
      }
    };
    close(SRC);
    # pass 2
    do {
      local($op, $label);
      @main'progseg = ();
      foreach (@progseg) {
	if (length($_) > 4) {
	  ($op, $lineno, $label) = unpack("a2sa*", $_);
	  $_ = $labels{$label};
	  &asmfatal("unknown label `$label'") unless (defined($_));
	  $_ = $op . $_;
	}
	push(@main'progseg, $_);
      };
      @progseg = ();
    };
  }

  sub main'write_code {
    local($objfile) = @_;
    $objfile = '-' unless (defined($objfile));
    open(OBJ, ">$objfile")
	  || &main'fatal("can't open object file `$objfile'\n");
    print OBJ $main'magic_number;
    foreach (@main'progseg) {
      print OBJ $_;
    }
    close(OBJ);
  }

  sub type1 { # OP
    local($op, $rest) = @_;
    &musteoln($rest);
    return &compose($op, 0, 0, 0, '');
  }

  sub type2 { # OP REG
    local($op, $rest) = @_;
    local($regno);
    ($regno, $rest) = &getregno($rest);
    &musteoln($rest);
    return &compose($op, 0, $regno, 0, '');
  }

  sub type3 { # OP REG, REG
    local($op, $rest) = @_;
    local($regx, $regy);
    ($regx, $rest) = &getregno($rest);
    $rest = &skipcomma($rest);
    ($regy, $rest) = &getregno($rest);
    &musteoln($rest);
    return &compose($op, 0, $regx, $regy, '');
  }

  sub type4 { # OP REG, {ADDR | ADDR(BR) | LABEL}
    local($op, $rest) = @_;
    local($regno, $addr, $mode);
    ($regno, $rest) = &getregno($rest);
    $rest = &skipcomma($rest);
    ($addr, $mode, $rest) = &getaddr($rest);
    &musteoln($rest);
    return &compose($op, $mode, $regno, 0, $addr);
  }

  sub type5 { # OP REG, IMMED
    local($op, $rest) = @_;
    local($regno, $immed);
    ($regno, $rest) = &getregno($rest);
    $rest = &skipcomma($rest);
    ($immed, $rest) = &getimmed($rest);
    &musteoln($rest);
    return &compose($op, 0, $regno, 0, &half($immed));
  }

  sub type6 { # OP LABEL
    local($op, $rest) = @_;
    local($label);
    ($label, $rest) = &getlabel($rest);
    &musteoln($rest);
    return &compose($op, 0, 0, 0, $label);
  }

  sub pseudo {
    local($op, $rest) = @_;
    if ($op eq 'data') {
      local($data);
      ($data, $rest) = &getimmed($rest, 1);
      if (defined($data)) {
	&musteoln($rest);
	return pack("i", $data);
      } else {
	($data, $rest) = &getlabel($rest);
	&musteoln($rest);
	return &half(0) . $data;
      }
    } else {
      &asmfatal("illegal pseudo op `$op'");
    }
  }

  @dispatch = (*pseudo, *type1, *type2, *type3, *type4, *type5, *type6);

  sub encode {
    local($id, $rest) = @_;
    local(*func, $op) = ($dispatch[$main'optype[$id]], $main'opcode[$id]);
    return &func($op, $rest);
  }

# get components

  sub getident {
    if ($_[0] =~ /^([\$A-Za-z_.][\$\w.]*)\s*/) {
      return ($1, $');
    }
    return (undef, $_[0]) if ($_[1]);
    &asmfatal("label or opcode was expected");
  }

  sub getregno {
    if ($_[0] =~ /^([Rr][0-5]|sp|SP|br|BR)\s*/) {
      return ($main'regno{$1}, $');
    }
    &asmfatal("reg name was expected");
  }

  sub getaddr {
    local($addr, $rest) = &getlabel($_[0], 1);
    if (defined($addr)) {
      return ($addr, 1, $rest);
    } elsif ($rest =~ /^(-?\d+)\s*/) {
      ($addr, $rest) = ($1, $');
      $rest = &skipblanks($rest);
      if ($rest =~ /^\(\s*(BR|br)\s*\)\s*/) {
	return (&half($addr), 2, $');
      } else {
	return (&half($addr), 0, $rest);
      }
    }
    &asmfatal("address or label was expected");
  }

  sub getlabel {
    local($label, $rest) = &getident($_[0], 1);
    if (defined($label)) {
      if (defined($labels{$label})) {
	return ($labels{$label}, $rest);
      } else {
	return (pack("s", $lineno) . $label, $rest);
      }
    }
    return (undef, $rest) if ($_[1]);
    &asmfatal("label operand was expected");
  }

  sub getimmed {
    if ($_[0] =~ /^(-?\d+)\s*/) {
      return ($1, $');
    } elsif ($_[0] =~ /^(-?)'(\\?.)'\s*/) {
      return (eval('ord("' . $2 . '")'), $');
    }
    return (undef, $_[0]) if ($_[1]);
    &asmfatal("immediate value was expected");
  }

  sub skipblanks {
    local($rest) = @_;
    $rest =~ s/^\s+//;
    return '' if ($rest =~ /^;/);
    return $rest;
  }

  sub skipcomma {
    if ($_[0] =~ /^[,\s]\s*/) {
      return $';
    }
    &asmfatal("operands should be separated by COMMA or SPACE");
  }

  sub musteoln {
    &asmfatal("end-of-line was expected") if (&skipblanks($_[0]) ne '');
  }

  sub compose {
    local($op, $mode, $rx, $ry, $addr) = @_;
    $addr = pack("s", 0) if ($addr eq '');
    return $op . pack("C", int(($mode << 6) | ($rx << 3) | $ry)) . $addr;
  }

  sub half {
    local($val) = @_;
    &asmfatal("overflow") if ($val <= -(1 << 16) || $val >= (1 << 16));
    return pack("s", $val);
  }

  sub asmfatal {
    &main'fatal("$lineno: ", @_, "\n");
  }
} # end of init_asm

sub read_code {
  local($objfile) = @_;
  local($buf);
  $objfile = '-' unless (defined($objfile));
  open(OBJ, "<$objfile")
    || &fatal("can't open object file `$objfile'\n");
  read(OBJ, $buf, length($main'magic_number));
  &fatal("file type mismatch.\n") if ($buf ne $main'magic_number);
  @progseg = ();
  while (read(OBJ, $buf, 4)) {
    &fatal("unexpected EOF\n") if (length($buf) < 4);
    push(@progseg, $buf);
  }
  close(OBJ);
}

sub init_disasm {
  package disasm;

  sub main'disasm_code {
    local($outfile) = @_;
    local($labelctr, $op, $type, $id, $mode, $regx, $regy, $addr, $buf);
    $outfile = '-' unless (defined($outfile));
    open(OUT, ">$outfile")
	  || &main'fatal("can't open output file `$outfile'\n");
    $labelctr = 0;
    foreach (@main'progseg) {
      print OUT &makelabel($labelctr), ": ";
      print OUT &decode($_), "\n";
      $labelctr++;
    }
    print OUT "; end\n";
    close(OUT);
  }

  sub decode {
    local($code) = @_;
    local($buf) = sprintf("data %11d", unpack("i", $code));
    local($id, $mode, $regx, $regy, $addr) = &sr'decompose($code, 1);
    if (defined($id)) {
      $buf .= "      ; " . pack("A8", $main'nmemonic[$id]);
      $type = $main'optype[$id];
      if ($type == 1) {
	;
      } elsif ($type == 2) {
	$buf .= $main'regname[$regx];
      } elsif ($type == 3) {
	$buf .= "$main'regname[$regx], $main'regname[$regy]";
      } elsif ($type == 4) {
	$buf .= "$main'regname[$regx], ";
	if ($mode == 1) {
	  $buf .= &makelabel($addr);
	} else {
	  $buf .= $addr;
	  $buf .= '(BR)' if ($mode == 2);
	}
      } elsif ($type == 5) {
	$buf .= "$main'regname[$regx], $addr";
      } elsif ($type == 6) {
	$buf .= &makelabel($addr);
      }
    }
    return $buf;
  }

  sub makelabel {
    return sprintf("L%05d", $_[0]);
  }
}

sub init_sr {
  package sr;

  sub init_regs {
    @dataseg = ();
    @progseg = ();
    $r0 = &gomi;
    $r1 = &gomi;
    $r2 = &gomi;
    $r3 = &gomi;
    $sp = &pint(-1);
    $br = &pint(0);
    @regp = (*r0, *r1, *r2, *r3, *sp, *br);
    $pc = 0;
    $cr = 0;
    $cycle = 0;
    $ppc = 0;
  }

  &init_regs;
  $curcode = undef;

  sub main'exec_code {
    local($step_mode) = $main'OPTIONS{'s'};
    local($trace_mode) = $main'OPTIONS{'t'} || $step_mode;
    &init_regs;
    $SIG{'INT'} = "sr'interrupt";
    while ($pc >= 0) {
      $curcode = $progseg[$pc];
      if (! defined($curcode)) {
	$curcode = &decode($main'progseg[$pc]);
	&illegal_insn if ($curcode eq '');
	$progseg[$pc] = $curcode;
      }
      &show_status if ($trace_mode);
      &prompt if ($step_mode);
      $ppc = $pc++;
      &execute($curcode);
    }
    print "\nHALT at $ppc.\n";
    print ";;; exec time = $cycle micro sec.\n";
    print ";;; prog segment size = ", scalar(@main'progseg), " words.\n";
    print ";;; required data segment size = ", scalar(@dataseg), " words.\n";
    $SIG{'INT'} = 'DEFAULT';
  }

  sub show_status {
    printf("\n%s: %s\n",
	   &disasm'makelabel($pc), &disasm'decode($main'progseg[$pc]));
    printf(";; R0=%d  R1=%d  R2=%d  R3=%d\n",
	   &upint($r0), &upint($r1), &upint($r2), &upint($r3));
    printf(";; SP=%d  BR=%d  CR=%d\n", &upint($sp), &upint($br), $cr);
  }

  sub prompt {
    print ";; press return key to continue:";
    <STDIN>;
  }

  sub execute {
    local($id, $mode, $regx, $regy, $addr) = split(/:/, $_[0], 5);
    local($type) = $main'optype[$id];
    if ($type == 1) {
	    &do_type1($id);
    } elsif ($type == 2) {
	    &do_type2($id, $regp[$regx]);
    } elsif ($type == 3) {
	    &do_type3($id, $regp[$regx], $regp[$regy]);
    } elsif ($type == 4) {
	    &do_type4($id, $regp[$regx], $mode, $addr);
    } elsif ($type == 5) {
	    &do_type5($id, $regp[$regx], $addr);
    } elsif ($type == 6) {
	    &do_type6($id, $addr);
    } else {
	    &illegal_insn;
    }
  }

  sub do_type1 {
    local ($id) = @_;
    local ($nmem) = $main'nmemonic[$id];
    if ($nmem eq 'return') {
      $pc = &upint(&do_pop);
    } elsif ($nmem eq 'halt') {
      $pc = -100;
    } else {
      &illegal_insn;
    }
    $cycle += $main'execycle[$id];
  }

  sub do_type2 {
    local ($id, *reg) = @_;
    local ($nmem) = $main'nmemonic[$id];
    if ($nmem eq 'push') {
      &do_push($reg);
    } elsif ($nmem eq 'pop') {
      $reg = &do_pop;
    } elsif ($nmem eq 'read') {
      $reg = &pint(int(<STDIN>));
    } elsif ($nmem eq 'writec') {
      print pack("C", &upint($reg));
    } elsif ($nmem eq 'writed') {
      print &upint($reg);
    } else {
      &illegal_insn;
    }
    $cycle += $main'execycle[$id];
  }

  sub do_type3 {
    local ($id, *regx, *regy) = @_;
    local ($nmem) = $main'nmemonic[$id];
    if ($nmem eq 'loadr') {
      $regx = $regy;
    } elsif ($nmem eq 'addr') {
      $regx = &pint(&upint($regx) + &upint($regy));
    } elsif ($nmem eq 'subr') {
      $regx = &pint(&upint($regx) - &upint($regy));
    } elsif ($nmem eq 'mulr') {
      $regx = &pint(&upint($regx) * &upint($regy));
    } elsif ($nmem eq 'divr') {
      $regx = &pint(&upint($regx) / &upint($regy));
    } elsif ($nmem eq 'cmpr') {
      $cr = &upint($regx) <=> &upint($regy);
    } else {
      &illegal_insn;
    }
    $cycle += $main'execycle[$id];
  }

  sub do_type4 {
    local ($id, *reg, $mode, $addr) = @_;
    local ($nmem) = $main'nmemonic[$id];
    local(*seg) = ($mode == 1) ? *main'progseg : *dataseg;
    if ($mode == 2) {
      $addr += &upint($br);
      $cycle += 2;
    }
    if ($nmem eq 'load') {
      $reg = $seg[$addr];
    } elsif ($nmem eq 'store') {
      $seg[$addr] = $reg;
      if ($mode == 1 && defined($progseg[$addr])) {
	$progseg[$addr] = undef;
      }
    } elsif ($nmem eq 'add') {
      $reg = &pint(&upint($reg) + &upint($seg[$addr]));
    } elsif ($nmem eq 'sub') {
      $reg = &pint(&upint($reg) - &upint($seg[$addr]));
    } elsif ($nmem eq 'mul') {
      $reg = &pint(&upint($reg) * &upint($seg[$addr]));
    } elsif ($nmem eq 'div') {
      $reg = &pint(&upint($reg) / &upint($seg[$addr]));
    } elsif ($nmem eq 'cmp') {
      $cr = &upint($reg) <=> &upint($seg[$addr]);
    } else {
      &illegal_insn;
    }
    $cycle += $main'execycle[$id];
  }

  sub do_type5 {
    local ($id, *reg, $immed) = @_;
    local ($nmem) = $main'nmemonic[$id];
    if ($nmem eq 'loadi') {
      $reg = &pint($immed);
    } elsif ($nmem eq 'addi') {
      $reg = &pint(&upint($reg) + $immed);
    } elsif ($nmem eq 'subi') {
      $reg = &pint(&upint($reg) - $immed);
    } elsif ($nmem eq 'muli') {
      $reg = &pint(&upint($reg) * $immed);
    } elsif ($nmem eq 'divi') {
      $reg = &pint(&upint($reg) / $immed);
    } elsif ($nmem eq 'cmpi') {
      $cr = &upint($reg) <=> $immed;
    } else {
      &illegal_insn;
    }
    $cycle += $main'execycle[$id];
  }

  sub do_type6 {
    local ($id, $label) = @_;
    local ($nmem) = $main'nmemonic[$id];
    if ($nmem eq 'jmp') {
      $pc = $label;
    } elsif ($nmem eq 'jnz') {
      $pc = $label if ($cr != 0);
    } elsif ($nmem eq 'jz') {
      $pc = $label if ($cr == 0);
    } elsif ($nmem eq 'jgt') {
      $pc = $label if ($cr > 0);
    } elsif ($nmem eq 'jge') {
      $pc = $label if ($cr >= 0);
    } elsif ($nmem eq 'jlt') {
      $pc = $label if ($cr < 0);
    } elsif ($nmem eq 'jle') {
      $pc = $label if ($cr <= 0);
    } elsif ($nmem eq 'call') {
      &do_push(&pint($pc));
      $pc = $label;
    } else {
      &illegal_insn;
    }
    $cycle += $main'execycle[$id];
  }

  sub do_push {
    $sp = &upint($sp) + 1;
    $dataseg[$sp] = $_[0];
    $sp = &pint($sp);
  }

  sub do_pop {
    local($val);
    $sp = &upint($sp);
    $val = $dataseg[$sp];
    $sp = &pint($sp - 1);
    return $val;
  }

  sub decode {
    return join(":", &decompose($_[0]));
  }

  sub decompose {
    local($code, $contp) = @_;
    local($id, $mode, $regx, $regy, $addr, $op, $m_n_r);
    &illegal_insn if (length($code) != 4);
    ($op, $m_n_r, $addr) = unpack("aCs", $code);
    $id = $main'codeid{$op};
    if (! defined($id)) {
      return () if ($contp);
      &illegal_insn;
    }
    ($mode, $regx, $regy) = split(//, sprintf("%03o", $m_n_r), 3);
    return ($id, $mode, $regx, $regy, $addr);
  }

  sub pint {
    return pack("i", $_[0]);
  }

  sub upint {
    return unpack("i", $_[0]);
  }

  sub gomi {
    return &pint(rand(1 << 16));
  }

  sub interrupt {
    &fatal("interrupt.\n(exec time = $cycle micro sec.)");
  }

  sub illegal_insn {
    &fatal("illegal instruction");
  }
  
  sub fatal {
    &main'fatal("$ppc: ", @_, "\n");
  }
}

sub fatal {
	print STDERR @_;
	exit(1);
}

sub init_op_definition {
	local($nextopid) = 0;
	&defop('load',   4, '00000010',  5);
	&defop('loadr',  3, '00000101',  1);
	&defop('loadi',  5, '00000111',  3);
	&defop('store',  4, '01110010',  5);
	&defop('add',    4, '00010010',  6);
	&defop('addr',   3, '00010101',  2);
	&defop('addi',   5, '00010111',  4);
	&defop('sub',    4, '00100010',  6);
	&defop('subr',   3, '00100101',  2);
	&defop('subi',   5, '00100111',  4);
	&defop('mul',    4, '00110010', 10);
	&defop('mulr',   3, '00110101',  6);
	&defop('muli',   5, '00110111',  8);
	&defop('div',    4, '01000010', 10);
	&defop('divr',   3, '01000101',  6);
	&defop('divi',   5, '01000111',  8);
	&defop('cmp',    4, '01010010',  6);
	&defop('cmpr',   3, '01010101',  2);
	&defop('cmpi',   5, '01010111',  4);
	&defop('jmp',    6, '10000000', 12);
	&defop('jnz',    6, '10001000', 12);
	&defop('jz',     6, '10000110', 12);
	&defop('jgt',    6, '10001100', 12);
	&defop('jge',    6, '10000100', 12);
	&defop('jlt',    6, '10001010', 12);
	&defop('jle',    6, '10000010', 12);
	&defop('call',   6, '10010000', 25);
	&defop('return', 1, '10010010', 15);
	&defop('push',   2, '10100000',  6);
	&defop('pop',    2, '10100010',  6);
	&defop('read',   2, '10110000',  0);
	&defop('writec', 2, '10110010',  0);
	&defop('writed', 2, '10110100',  0);
	&defop('halt',   1, '00000000',  0);

	# pseudo instructions
	&defpseudo('data');
}

sub defop {
	local($nmem, $type, $code, $cycle) = @_;
	$code = pack("B8", $code);
	return 0 if ($nmemid{$nmem} ne '' || $codeid{$code} ne '');
	$nmemid{$nmem} = $nextopid;
	$codeid{$code} = $nextopid;
	$nmemonic[$nextopid] = $nmem;
	$optype[$nextopid] = $type;
	$opcode[$nextopid] = $code;
	$execycle[$nextopid] = $cycle;
	$nextopid++;
	return 1;
}

sub defpseudo {
	local($nmem) = @_;
	return 0 if ($nmemid{$nmem} ne '');
	$nmemid{$nmem} = $nextopid;
	$nmemonic[$nextopid] = $nmem;
	$optype[$nextopid] = 0;
	$opcode[$nextopid] = $nmem;
	$execycle[$nextopid] = 0;
	$nextopid++;
	return 1;
}

### Local Variables: ###
### mode:perl ###
### End: ###
