#!/usr/bin/perl -w

## use lib "/home/ansell/exe/getMk";
use lib "./";

use CMakeList;
use strict;

## EXECUTABLES
my @masterprog=("fullBuild","ess","muBeam","pipe","photonMod2","t1Real",
		"sns","saxs","reactor","t1MarkII","essBeamline","bilbau",
		"singleItem","maxiv","testMain"); 



## Model Directory
##my @modelLibDir=qw( bibBuild bnctBuild build chip 
##                    cuBlock d4cModel delft epbBuild essBuild
##                    gammaBuild imat lensModel moderator 
##                    muon pipeBuild photon sinbadBuild snsBuild t1Build 
##                    t1Engineer t1Upgrade t3Model zoom );

##my @modelNames= @modelLibDir;

## INCLUDES

## MODEL Directory
my @modelInclude = qw( bibBuildInc bnctBuildInc buildInc chipInc 
                       cuBlockInc d4cModelInc delftInc epbBuildInc 
                       essBuildInc gammaBuildInc imatInc lensModelInc 
                       moderatorInc muonInc pipeBuildInc photonInc
                       singleItemBuildInc sinbadBuildInc snsBuildInc t1BuildInc 
                       t1EngineerInc t1UpgradeInc t3ModelInc zoomInc );


## SYSTEM Directory
my @systemInclude = qw(
                     attachCompInc constructInc compWeightsInc crystalInc 
                     endfInc flukaProcessInc flukaPhysicsInc
                     flukaTallyInc funcBaseInc geomInc 
                     inputInc logInc md5Inc 
                     mersenneInc monteInc 
                     phitsProcessInc phitsPhysicsInc
                     physicsInc polyInc processInc 
                     simMCInc sourceInc supportInc tallyInc 
                     visitInc weightsInc worldInc workInc xmlInc 
                     );

my @incdir=qw( include beamlineInc globalInc instrumentInc 
               scatMatInc specialInc transportInc testInclude );


my @mainLib=qw( visit src simMC  construct physics input process 
    transport scatMat endf crystal source monte funcBase log monte 
    flukaProcess flukaPhysics 
    flukaTally phitsProcess  
    phitsTally phitsSupport tally 
    geometry mersenne src world work 
    xml poly support weights 
    insertUnit md5 construct 
    global constructVar physics simMC transport 
    attachComp visit poly flukaMagnetic phitsPhysics
    scatMat endf crystal);

my $gM=new CMakeList;
$gM->setParameters(\@ARGV);
$gM->addMasterProgs(\@masterprog);
$gM->addIncDir("",\@incdir);
$gM->findSubIncDir("System");
$gM->findSubIncDir("Model");
$gM->findSubIncDir("Model/ESSBeam");
$gM->findSubIncDir("Model/MaxIV");

$gM->findSubSrcDir("Model");
$gM->findSubSrcDir("System");
$gM->findSubSrcDir("Model/ESSBeam");
$gM->findSubSrcDir("Model/MaxIV");
print STDERR "ERER \n";
$gM->findSubSrcDir("","Aunit");

my @ess = qw( essBuild beer input input  process  source  funcBase  
              log  monte  flukaProcess  flukaPhysics  
              flukaTally  phitsProcess  phitsTally  
              phitsSupport  tally  geometry  mersenne  
              src  world  work  xml  support  weights  
              insertUnit  md5  construct  global  
              constructVar  physics  simMC  transport  
              attachComp  visit  poly  flukaMagnetic  
              phitsPhysics  scatMat  endf  crystal  
              essConstruct  commonVar  common  bifrost  
              cspec  dream  estia  freia  heimdal  loki  
              magic  miracles  nmx  nnbar  odin  skadi  
              testBeam  trex  vor  vespa  shortOdin  
              shortNmx  shortDream  simpleItem  beamline  
              instrument );

$gM->addDepUnit("ess", [@ess]);
 

my @essBeam = qw( essBuild beer input process source funcBase 
                  log monte flukaProcess flukaPhysics flukaTally 
                  phitsProcess phitsTally phitsSupport tally geometry 
                  mersenne src world work xml support weights insertUnit 
                  md5 construct global physics simMC transport attachComp 
                  visit poly flukaMagnetic phitsPhysics scatMat endf crystal 
                  common bifrost cspec dream estia freia heimdal loki magic 
                  miracles nmx nnbar odin skadi testBeam trex vor vespa 
                  shortOdin shortNmx shortDream simpleItem instrument 
                  essConstruct commonVar constructVar beamline );
$gM->addDepUnit("essBeamline", [@essBeam]);


my @maxiv = qw( maxivBuild balder cosaxs danmax input process 
                source funcBase log monte flukaProcess flukaPhysics 
                flukaTally phitsProcess phitsTally phitsSupport tally 
                geometry mersenne src world work xml support weights 
                insertUnit md5 construct global constructVar physics 
                simMC transport attachComp visit poly flukaMagnetic 
                phitsPhysics scatMat endf crystal flexpes formax maxpeem 
                micromax commonGenerator commonBeam R3Common R1Common species );
$gM->addDepUnit("maxiv", [@maxiv]);


my @filter = qw( filter photon );
push(@filter,@mainLib);
$gM->addDepUnit("filter", [@filter]),
			   

my @bilbau = qw( bibBuild );
push(@bilbau,@mainLib);
$gM->addDepUnit("bilbau", [@bilbau]),
			   			   
my @fullBuild = qw( moderator build chip build zoom imat );
push(@fullBuild,@mainLib);
$gM->addDepUnit("fullBuild", [@fullBuild]),

my @d4c = qw( d4cModel ) ;
push(@d4c,@mainLib);
$gM->addDepUnit("d4c", [@d4c]);

my @t3Expt = qw( t3Model ); 
push(@t3Expt,@mainLib);
$gM->addDepUnit("t3Expt", [@t3Expt]);

my @lens = qw( lensModel ) ;
push(@lens,@mainLib);
$gM->addDepUnit("lens", [@lens]);

$gM->addDepUnit("simple", [@mainLib]);

my @t1MarkII = qw( t1Upgrade t1Build build moderator imat chip zoom  ) ;
push(@t1MarkII,@mainLib);
$gM->addDepUnit("t1MarkII", [@t1MarkII]);

my @t1Eng = qw( t1Engineer t1Upgrade t1Build build moderator imat chip zoom  ) ;
push(@t1Eng,@mainLib);
$gM->addDepUnit("t1Eng", [@t1Eng]);

my @photonMod = qw( photon ) ;
push(@photonMod,@mainLib);
$gM->addDepUnit("photonMod", [@photonMod]);

my @photonMod2 = qw( photon ) ;
push(@photonMod2,@mainLib);
$gM->addDepUnit("photonMod2", [@photonMod2]);

my @photonMod3 = qw( photon ) ;
push(@photonMod2,@mainLib);
$gM->addDepUnit("photonMod3", [@photonMod3]);

my @pipe = qw( pipeBuild ) ;
push(@pipe,@mainLib);
$gM->addDepUnit("pipe", [@pipe]);

my @singleItem = qw( singleItemBuild commonVar commonGenerator R3Common ) ;
push(@singleItem,@mainLib);
$gM->addDepUnit("singleItem", [@singleItem,
			       qw( commonBeam  )]);

my @t1Real = qw( t1Build build imat moderator chip zoom ) ;
push(@t1Real,@mainLib);
$gM->addDepUnit("t1Real", [@t1Real]);


my @reactor = qw( delft ) ;
push(@reactor,@mainLib);
$gM->addDepUnit("reactor", [@reactor]);


$gM->addDepUnit("siMod", \@mainLib);

my @cuBuild = qw( cuBlock delft ) ;
push(@cuBuild,@mainLib);
$gM->addDepUnit("cuBuild", [@cuBuild]);


my @sinbad = qw( sinbadBuild ) ;
push(@sinbad,@mainLib);
$gM->addDepUnit("sinbad", [@sinbad]);


my @sns = qw( snsBuild ) ;
push(@sns,@mainLib);
$gM->addDepUnit("sns", [@sns]);

my @epb = qw( epbBuild ) ;
push(@epb,@mainLib);
$gM->addDepUnit("epb", [@epb]);

my @muBeam = qw( muon ) ;
push(@muBeam,@mainLib);
$gM->addDepUnit("muBeam", [@muBeam]);


my @gamma = qw( gammaBuild ) ;
push(@gamma,@mainLib);
$gM->addDepUnit("gamma", [@gamma]);

my @saxs = qw( saxs d4cModel instrument );
push(@saxs,@mainLib);
$gM->addDepUnit("saxs", [@saxs]),
			   

my @testMain = qw( test ) ;
push(@testMain,@mainLib);
$gM->addDepUnit("testMain", [@testMain]);


$gM->writeCMake();

print "FINISH CMake.pl\n";
exit 0;

