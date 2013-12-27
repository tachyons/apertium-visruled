#How to Use the Apertium Visual Rule Editor

##1. Introduction

###1.1 What is Apertium?

Apertium is a machine translation toolbox. A set of programs that, if provided with certain dictionaries and files, can translate text between languages. Apertium is a *rule-based translator*, meaning that you have to tell it how to reorder words, add or remove affixes etc. to make a proper translation - these are the so-called *transfer rules*. (Contrarily to *stastistical translators*, who figure that out themselves by examining large blocks of text).

###1.2 What is Visruled?

Visruled is an application to write and edit Apertium transfer rules. Please note that (as of now) Visruled only offers a limited subset of Apertium's features, and it's not really capable of handling real-world Apertium rule files. Visruled is for beginners who want to familiarize with the world of machine translation, or want to start a language pair but lack the necessary technical skills. It is by no means a professional tool.

Visruled is only for editing transfer rules, you can't create dictionaries with it. If you don't know how to do that, see the first few sections of the [New Language Pair Howto](http://wiki.apertium.org/wiki/Apertium_New_Language_Pair_HOWTO).

##2. Setting up the environment

###2.1 Install Apertium and lttoolbox

See instructions on the [wiki](http://wiki.apertium.org/wiki/Installation).

###2.2 Install Visruled

####Unix/Linux

Open a termial in the source directory, and type:

	make
	sudo make install

You'll need the Qt 4 development package to compile Visruled.

###2.3 Create a new t1x file

Find the "New..." option in the files menu and from the pop-up dialog choose "Transfer (.t1x)". (Not as if there were any other options. Click OK. Now you should see a vertical tab opened with the title "New file\*" and four horizontal tabs: Categories, Attributes, Variables and Rules. To work with this file, you'll also need to specify the location of your dictionaries. You may do so by opening the settings dialog from the "Edit" menu and entering the correct values at three text entries at the bottom.

##3. Working with Visruled

###3.1 Morphological analysis

As a first step in the translation procedure, Apertium will perform a so-called *morphological analysis* on each word. This determines what case, number, person etc. the word is in. To see how this works, click "Compile" from the "Tools" menu and then "Test..." from "Tools" again. Check the "Morphological analysis" radio button, then enter some word in the upper text box (make sure it's one that's present in your source language dictionary). For the rest of this tutorial we will use the English-French pair as an example. So let's type, for example, "cats" and then click the "Go!" button. We see this in the lower text area:

	cat<noun><pl>

At the beginning we can see the lemma of the analysed word, with one or more *tags* following. Each tag tells us something about the word; in this case, we know that it's a noun and that it's in the plural form.

###3.2 Word categories

Let's close the test window for now and start working on the rules themselves. Firstly, we'll put the words into *categories*. Click on the categories tab, and then on the canvas (the white square in the middle). Now you should see a reddish "Category" box on the left, below the "Add child" label. Visruled diagrams are made up of *nodes* and *properties*. Nodes are basically boxes and properties are different input widgets. Whenever you click on a box (or the canvas) on the diagram, the sidebar will display all the child nodes and properties that you can add to that node.

Now move the mouse over the "Category" box, press the left button and (without releasing it) move the cursor over the canvas. Release the button, and shazamm! You have a category node on the canvas. You might also notice that a property, "Name", was automatically added to the node. Properties that come with the node are *mandatory properties*. While you can remove them like any other (more on that later), it's usually not a good idea.

Select the "Name" entry and type "noun". Now we have a word category called "noun", but don't yet have any words in it. Click on the category box on the canvas. Drag the "Tags:" node from the sidebar onto the the "Category" node. You'll see that there's now an embedded box inside "Category". We will use this node to tell Apertium what tags the items of the "noun" category have - or more more precisely, what tags make a word a noun.

Click on the newly added "Tags:" box. You'll see a number of blue boxes in the sidebar, whose names are enclosed in angle brackets (\< and \>). Find the \<noun\> box and add it to "Tags:". Now Apertium will know that any word that has exactly one \<noun\> tag is a "noun". Add another "Tags:" child to "Category", and again a "\<noun\>" to "Tags:". Then find the "anything" box on the sidebar, and drag it to "Tags:" as well, placing it after "\<noun\>". This means "one \<noun\> tag at the beginning, and then any number of other tags". If you make a mistake adding child nodes, you can reorder them via drag and drop, or simply remove them by right clicking on them and selecting "Delete child" from the context menu.

As practice, create an "adj" category in a similar fashion.

###3.3 Attributes

After we have categorized words, we're going to categorize tags as well. Certain tags have the same purpose - for example, \<sg\> and \<pl\> both describe the number of the word (singular or plural, respectively). We'll refer to such tag groups as *attributes*. Defining attributes isn't much different from defining categories. Create an "Attribute" node, and give it the name "a\_nbr". For "Attribute", there's no "Tags" child, you may add the tags directly. Place \<sg> and \<pl> into this node. Since French has genders, we'll also create an "a\_gnd" attribute with the children \<m\> and \<f\>. Finally, two more attributes are required, a\_noun and a\_adj to describe the "nounness" and "adjectiveness" of words. Both of them contains only one tag, \<noun\> and \<adj\>.

###3.4 Simple transfer

Finally, we can start writing rules! Switch to the last tab ("Rules"), and place a "Rule" box on the canvas. For each rule, we'll define a pattern of words on which the rule is applicable to.

In this tutorial, we'll use a simple adjectival construction as an example. Add a "Pattern" child to the rule, and then to "Item" nodes. Give the first item's "Word category" property the value "adj", and "noun" to the second's. This restricts the rule's scope to two-word structures, where the first word is of the "adj" and the second is of the "noun" category. Now select "Rule" again, and add an "Action" child (this node isn't embedded into the parent, instead they're connected with an arrow). "Action" contains all the reorderings, insertions and other operations we perform on the word structure to translate it to the target language. 

You can use the following operations:
* *Swap words:* has two properties, "Word 1" and "Word 2" which refer to positions of words within the structure. The operation swaps the positions of words, so "Word 1" becomes "Word 2" and vica versa.
* *Agree attribute:* copies any tags of the specified attribute from the source word to the target word
* *Remove word:* removes a word from the given position
* *Reorder attributes:* change the order of the tags of the word. The new order is given as a comma-separated list of attributes. Any attribute that isn't present in the new order is removed from the word.
* *Insert word:* insert the given word into the specified position
* *Insert tag:* append the given tag to the word at the specified position
* *Assign variable:* change the value of a variable... this is a bit more advanced stuff.

In a French adjectival construction, the adjective inherits the gender and the number of the noun (unlike English, where adjectives don't have number nor gender). So we'll use "Agree attribute" to copy the relevant tags.

1. Agree attribute, source word: 2, target word: 1, attribute: a\_gnd
2. Agree attribute, source word: 2, target word: 1, attribute: a\_nbr

Unfortunately, Apertium requires tags in the translated construction to be in the same order as in the morphological dictionary - otherwise it won't generate the result properly. So we use "Reorder attributes".

3. Reorder attributes, word: 1, order: a\_adj,a\_nbr,a\_gnd
4. Reorder attributes, word: 2, order: a\_noun,a\_nbr,a\_gnd

And finally, let's swap the two words. (In a regular French adjectival construction, the noun comes first).

5. Swap words, word 1: 1, word 2: 2

That's all! Hit "Compile" again, then open the test dialog, switch to "Transfer and generate" and translate "blue cats". The result is "chats bleus", as expected.

###3.5 Variables

You may have noticed that there's a fourth tab on the interface, we didn't discuss. It is for declaring variables - variables are containers that can be assigned a value. There's a variable called "number" declared by default (it's required by Apertium). You can create your own variables in a similar way.

To change a variable's value, use the "Assign variable" operation. It has one mandatory property, the variable's name. You can assign the following types of values:

* A literal. From the sidebar, drag the "Literal:" property onto the box, and enter the desired literal value.
* A tag. Same as above, but with "Literal tag:". You don't need to enter the angle brackets!
* Part of a word. Add both "Word:" and "Part:". "Word:" works as usual, taking the position of a word, "Part:" can either be an attribute name (this assigns a tag), or "lem" if you want to assign the word's lemma, or "whole" to use the whole word.

Visruled currently supports only one kind of usage of variables. With "Insert word" and "Insert tag", you can remove the default "Literal" property (right click on the label and "Delete child"), and instead add the "From variable" property from the sidebar. It takes the name of a variable and substitutes its value.

###3.6 Conditional structures

Sometimes, you may want a rule differently based on the input. You can achieve this with a *conditional structure*. Instead of adding the "Action" node directly to "Rule", add instead a "Choose". "Choose" has two types of children, "When" and "Otherwise". "When" may include condition nodes, such as:

* Equals: checks if two items are identical. The first item (defined by "Word 1:" and "Part 1:") is always a part of a word, the second item is wither a part of a word ("Word 2:", "Part 2:"), a literal ("Literal:") or a tag ("Literal tag:").
* Begins with: checks if the first item begins with the second item. Items are defined identically as in "Equals".
* Ends with: checks if the first item ends with the second item.

An "Action" child may (and ought to) be added to both "When" and "Otherwise". The action sequence of a "When" node is performed only if its conditions are fulfilles. An "Otherwise" sequence is executed if none of the "When" conditions were fulfilled.
