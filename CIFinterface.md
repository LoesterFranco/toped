<p align='right'><b>Disclaimer:</b> All quoted or linked documents below are (c) Copyright of their respective owners. This article is not an authoritative description neither of the CIF format nor of its user extensions<br>
</p>

<p align='right'> <b>09/11/2008</b> </p>


---


# Toped CIF interface #

[Caltech Intermediate form](http://en.wikipedia.org/wiki/Caltech_Intermediate_Form) (CIF) is a low level graphics language for specifying the geometry of integrated circuits. Toped CIF interface is written to comply with CIF 2.0 definition as described in "[A Guide to LSI implementation, Second Edition](http://ai.eecs.umich.edu/people/conway/VLSI/ImplGuide2/ImplGuide2ndEd.html)" by Robert W. Hon and Carlo H.Sequin. The format seem to be naturally exposed to different interpretation and user expansion which could be a reason for bitter troubles with the data transfers between the tools. This page describes in more details the CIF parser and generator as implemented in [Toped](http://www.toped.org.uk/trm_ifaces.html#CIF). The [latest source code](http://code.google.com/p/toped/source/browse/trunk/tpd_ifaces/cif_io.cpp) itself can be browsed on-line.

---


## The CIF parser ##
The parser is implemented using [flex](http://dinosaur.compilertools.net/flex/index.html) and [bison](http://www.gnu.org/software/bison/bison.html). The formal syntax in _Wirt_ notation as given in the quoted paper is given below. Some important features of the CIF syntax:
  * CIF uses key characters instead of key words
  * CIF doesn't define a separator, it defines blank instead. In more relaxed terms blank is everything that is not a key character or meaningful token in the current context. This is one of the features that effectively introduces _CIF slangs_. As defined in the paper _" ... syntax allows blanks before and after commands, and blanks or other kinds of separators (almost any character) before integers etc."_
  * As opposite to GDSII, CIF defines layer identificators as strings and cell identificators as unsigned integers.

### Formal syntax ###
```
cifFile              = {{blank} [command] semi} endCommand {blank}.

command              = primCommand | defDeleteCommand |
                       defStartCommand semi {{blank}[primCommand]semi} defFinishCommand.

primCommand          = polygonCommand       | boxCommand    | roundFlashCommand |
                       wireCommand          | layerCommand  | callCommand       |
                       userExtensionCommand | commentCommand

polygonCommand       = "P" path.
boxCommand           = "B" integer sep integer sep point [sep point].
roundFlashCommand    = "R" integer sep point.
wireCommand          = "W" integer sep path.
layerCommand         = "L" {blank} shortname.
defStartCommand      = "D" {blank} "S" integer [sep integer sep integer].
defFinishCommand     = "D" {blank} "F".
defDeleteCommand     = "D" {blank} "D" integer.
callCommand          = "C" integer transformation.
userExtensionCommand = digit userText
commentCommand       = "(" commentText ")".
endCommand           = "E".

transformation       = {{blank} ("T" point | "M" {blank} "X" | "M" {blank} "Y" | "R" point)}.
path                 = point {sep point}.
point                = sInteger sep sInteger.
sInteger             = {sep} ["-"]integerD.
integer              = {sep} integerD.
integerD             = digit{digit}.
shortname            = c[c][c][c]
c                    = digit | upperChar
userText             = {userChar}.
commentText          = {commentChar} | commentText "(" commentText ")" commentText.

semi                 = {blank};{blank}.
sep                  = upperChar | blank;
digit                = "0" | "1" | "2" | ... | "9"
upperChar            = "A" | "B" | "C" | ... | "Z"
blank                = any ASCII character except digit, upperChar, "-" , "(" , ")" , ";".
userChar             = any ASCII character except ";".
commentChar          = any ASCII character except "(" or ")".


```

### CIF user extensions ###
I couldn't find an authoritative description of the CIF user extensions. The [wikipedia](http://en.wikipedia.org/wiki/Caltech_Intermediate_Form) describes informally some of them. Generally the same description can be found also [here](http://www.rulabinsky.com/cavd/text/chapb.html). Some of the user extensions are used effectively in all foreign CIF files. The user extensions implemented currently in Toped are formally described below in the same terms as the original syntax.

```
cellName             = '9' userExtSeparator userExtString.
label                = '94' userSeparator userExtString blank point.
cellBoundary         = '4A' point sep point.
labelSignal          = '4N' userSeparator userExtString blank point.
userExtString        = {userExtChar}.
userExtChar          = any ASCII character except semicolon , comma or userSeparator.
userExtSeparator     = space or tab.

```
Note that user extensions are the only possibility to transfer text objects. This possibility however is quite limited, because it is transferring neither the font size nor the text placement properties (rotation, flip). User extensions are also the only possibility to transfer the cell names. There is no possibility though to translate the layer numbers.

### Translations ###
The translation of the most of the data is trivial. Here are the exceptions.
  * Cells - If the cell name is not stated using the user extension command, the cell name is generated using the simple scheme cell`<ID>`
  * Layers - [Layer maps](http://www.toped.org.uk/trm_ifaces.html#getciflaymap) has to be used during the translation in both directions
<span> </span>
### What is not translated ###
  * roundFlashCommand
  * defDeleteCommand
For both of them Toped issues a warning message in the log. This will be updated in the forthcoming revisions of Toped.


---


## The CIF generator ##
Toped generates two different CIF slangs.
  * The short one (which is the default) seems to be the most common CIF formatting and should be accepted by other EDA tools.
  * The verbose uses widely the concept of a **blank** as described in the quoted paper. It implements a formatting which is effectively as close as possible to the examples in that paper. It appears though that not all of the EDA vendors (including major one) accept this type of CIF formatting.

The examples below demonstrate the differences. Both of them are describing completely equivalent data
```
DS 1;
   9 Fcell;
L ACTI;
      P 2000 0 2000 4000 4000 4000 4000 6000 2000 6000 2000 8000 6000 8000 6000 10000 0 10000 0 0;
DF;

```

```
Definition Start #1;
   9 Fcell;
   Layer ACTI objects follow;
      Polygon with vertices 2000 0 2000 4000 4000 4000 4000 6000 2000 6000 2000 8000 6000 8000 6000 10000 0 10000 0 0;
Definition Finish;

```


---


## Unreadable CIF slangs ##
Although with quite free syntax, the CIF format still has some rules that must be followed. It appears that some EDA companies are making possible to break some of those syntax rules. One of the examples is generating _layer names in lower case_. It is impossible to distinguish them from the blanks. The syntax rules clearly state that a layer name can be up to four characters and all of them should be either upper case characters or digits.

This kind of changes require different parser rules. A parser however shall be based on a clearly defined set of rules. It can not be based on examples. At the moment I don't know a document including EDA vendor documents describing a version of the CIF format which allows layer names in lowercase. That's why those CIF slangs are considered unreadable for the current implementation. In most of the cases they can be quickly converted to a proper CIF syntax using simple text editing, nevertheless as stated above parser should be based on a formal rules and those kind of rules doesn't seem to exist.
<span>
</span>