<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fr_FR">
<context>
    <name>OpenNIApplication</name>
    <message>
        <location filename="../src/openniapplication.cpp" line="176"/>
        <source>OpenNI already initialized !</source>
        <translation>OpenNI déjà initialisé !</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="184"/>
        <source>Failed to attach to the depth shared memory ! Error: %1</source>
        <translation>Impossible de se lier avec la mémoire partagée pour les données de profondeur ! Erreur: %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="191"/>
        <source>Failed to attach to the information shared memory ! Error: %1</source>
        <translation>Impossible de se lier avec la mémoire partagée pour les informations de caméra ! Erreur: %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="195"/>
        <source>Initializing OpenNI ...</source>
        <translation>Initialisation d&apos;OpenNI ...</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="201"/>
        <source>Context Init</source>
        <comment>on error</comment>
        <translation>Initialisation du contexte</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="206"/>
        <source>Search available devices</source>
        <comment>on error</comment>
        <translation>Recherche de périphériques</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="216"/>
        <source>Creating device: %1</source>
        <translation>Création du périphérique: %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="219"/>
        <source>Create device</source>
        <comment>on error</comment>
        <translation>Création de périphérique</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="227"/>
        <source>Create depth generator</source>
        <comment>on error</comment>
        <translation>Création du générateur de profondeur</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="231"/>
        <source>Create user generator</source>
        <comment>on error</comment>
        <translation>Création du générateur utilisateur</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="235"/>
        <source>Register to user callbacks</source>
        <comment>on error</comment>
        <translation>Enregistrement des méthodes de rappel utilisateur</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="237"/>
        <source>Register to calibration start</source>
        <comment>on error</comment>
        <translation>Enregistrement des méthodes de rappel de début de calibration</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="239"/>
        <source>Register to calibration complete</source>
        <comment>on error</comment>
        <translation>Enregistrement des méthodes de rappel de fin de calibration</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="244"/>
        <source>Supplied user generator doesn&apos;t support skeleton capability.</source>
        <translation>Le générateur utilisateur fournit ne supporte pas la reconaissance du squelette.</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="250"/>
        <source>Pose calibration required but not supported by this program.</source>
        <translation>Une pose de calibration est requise mais n&apos;est pas supportée par le programme.</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="281"/>
        <source>The specified device doesn&apos;t exist !</source>
        <translation>Le périphérique spécifié n&apos;existe pas !</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="293"/>
        <source>The application is not initilized, can&apos;t start !</source>
        <translation>L&apos;application n&apos;est pas initialisée, impossible de la lancer !</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="297"/>
        <source>Starting OpenNI main loop ...</source>
        <translation>Lancement de la boucle principale d&apos;OpenNI ...</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="300"/>
        <source>Start Generating</source>
        <comment>on error</comment>
        <translation>Début de la génération</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="432"/>
        <location filename="../src/openniapplication.cpp" line="440"/>
        <source>Trying to use a Kinect functionnality without enabling the support !</source>
        <translation>Tentative d&apos;utiliser une fonctionnalité de la Kinect sans avoir activé son support !</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/main.cpp" line="70"/>
        <source>Sensor %1</source>
        <translation>Capteur %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="35"/>
        <source>%1 failed: %2</source>
        <comment>%1 is what failed and %2 is the error from OpenNI SDK.</comment>
        <translation>%1 a échoué: %2</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="43"/>
        <source>Cannot get the OpenNI application.</source>
        <translation>Impossible de récupérer l&apos;application OpenNI.</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="57"/>
        <source>New user: %1</source>
        <translation>Nouvel utilisateur: %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="65"/>
        <source>Lost user: %1</source>
        <translation>Perte de l&apos;utilisateur: %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="72"/>
        <source>Calibration started for user: %1</source>
        <translation>Début de la calibration pour l&apos;utilisateur: %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="83"/>
        <source>Calibration complete, start tracking user %1</source>
        <translation>Calibration finie, début du suivi de l&apos;utilisateur %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="89"/>
        <source>Calibration failed for user: %1</source>
        <translation>Erreur de calibration pour l&apos;utilisateur: %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="91"/>
        <source>Manual abort occured, stop attempting to calibrate !</source>
        <translation>Arrêt manuel demandé, fin de la calibration !</translation>
    </message>
    <message>
        <source>Stopping the OpenNI frame loop ...</source>
        <translation type="vanished">Arrêt de la boucle principale d&apos;OpenNI ...</translation>
    </message>
</context>
<context>
    <name>USBController</name>
    <message>
        <location filename="../src/usbcontroller.h" line="84"/>
        <source>The USB controller is already initialized !</source>
        <translation>Le contrôleur USB est déjà initialisé !</translation>
    </message>
    <message>
        <location filename="../src/usbcontroller.h" line="95"/>
        <source>Cannot open the usb device (%1). Error code: %3</source>
        <translation>Impossible d&apos;ouvrir le périphérique USB (%1). Code d&apos;erreur: %3</translation>
    </message>
    <message>
        <location filename="../src/usbcontroller.h" line="99"/>
        <source>USB device initialized (%1) !</source>
        <translation>Périphérique USB initialisé (%1) !</translation>
    </message>
    <message>
        <location filename="../src/usbcontroller.h" line="109"/>
        <source>The USB controller is not initialized !</source>
        <translation>Le contrôleur USB n&apos;est pas initialisé !</translation>
    </message>
    <message>
        <location filename="../src/usbcontroller.h" line="117"/>
        <source>Error when sending data to the USB device.</source>
        <translation>Erreur lors de l&apos;envoi de données au périphérique USB.</translation>
    </message>
    <message>
        <location filename="../src/usbcontroller.h" line="156"/>
        <source>You must specify an angle between -30° and 30°.</source>
        <translation>Vous devez spécifier un angle entre -30° et +30°.</translation>
    </message>
</context>
</TS>
