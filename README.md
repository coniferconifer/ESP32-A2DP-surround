# Surround Sound Experiment Using Speaker Matrix Method by ESP32-A2DP

In the 1970s, various methods were developed to attempt 5.1-channel surround sound conversion from 2-channel audio sources using the speaker matrix method. During that time, engineers devised creative wiring solutions to provide R-L signals to the right rear speaker and L-R signals to the left rear speaker. However, with today's power amplifiers utilizing Balanced Transformer Less (BTL) outputs, connecting speakers in a matrix configuration can be risky.



Personally, I use a 2.1-channel PC speaker system, where the ".1" refers to the dedicated subwoofer for low-frequency sounds. While I'm satisfied with this setup, I've always been curious about experiencing surround sound using the speaker matrix approach.

# Surround processor by OP-amp

To explore this, I built an OP-amp circuit capable of generating R-L and L-R signals similar to the speaker matrix method for rear speakers.
I connected this circuit to a rear speaker power amplifier (such as PAM8403 or ST893) and tried the resulting surround effect. 

Ref.surroundAMP.png surroundAMP.kicad_sch for Kicad schematic editor.

![](surroundAMP.png)


![](surroudAMPblock.png)


Indeed, the differential R-L signals allow playback of sounds originating from areas other than the stereo microphone's center, creating a wider soundstage during live playback or applause in 2-channel audio sources.

# MX-15 emulation by ESP32-A2DP

The famous designer Nagaoka Tetsuo implemented a matrix speaker system where the left speaker carried 2L-R signals, the center speaker played R+L, and the right speaker played 2R-L signals. I wonder what kind of audio experience this configuration would yield.

![](MX-15speaker.png)

To achieve this, I used the ESP32-A2DP library to receive Bluetooth audio and extract 2-channel stereo PCM data within callback functions. I then processed this data to create separate 2L-R and 2R-L signals for the left and right speakers. For the R+L signal intended for the center speaker, I used a simple analog mixer with two 2k-ohm resistors and a 100k-ohm resistor. The DAC (PCM5201A) was wired according to the standard GPIO usage of the ESP32-A2DP library.

![](MX15-emulator.png)

Keep in mind that a power amplifier is necessary to drive the speakers from the DAC output. Existing PC speakers are used to play 2L-R and 2R-L.  
For the center speaker, I used a small, inexpensive speaker with a built-in power amplifier. 
The result was a surround sensation, although not precisely 2.1-channel due to the unique configuration.

If you prefer to use Rear speakers,  L-R, R-L OP-amp surround processor is included , 2L-R and 2R-L are processed as 
<pre> 
Left channel (2L-R)-(2R-L) => 3*(L-R) 
Right channel (2R-L)-(2L-R) => 3*(R-L) 
</pre>
and surround processor by OP-amp works as well.

![](PCM5102A.JPG) DAC PCM5102A output has a splitter for 2.1ch PC speaker and small center speakers.

![](SmallCenterSpeakerforR+L.JPG) small powered speakers for center (mixed 2L-R and 2R-L are equivalent to R+L )

Referemces:

[1] https://www.pschatzmann.ch/home/2023/07/08/esp32-mixing-a2dp-with-a-sine-signal/

[2] https://github.com/pschatzmann/ESP32-A2DP

[3] MX-15 speaker by Tetsuo Nagaoka


In Japanese

# ESP32-A2DPによるスピーカマトリクス方式の音声受信

スピーカマトリクス法による2ch音源を5.1chサラウンド化する試みは1970年台にさまざまな方法が開発されていたそうです。
右側のリアスピーカにR-L信号,左のリアスピーカにL-R信号を与えるには、当時はスピーカーの配線を工夫することで実現されていましたが、
現在のパワーアンプはBTL(Balanced Transformer Less)出力のため、左右のスピーカのマイナス側は共通のGNDではないためスピーカーをマトリクスつなぎにするのは危険です。

現在2.1chのPCスピーカを愛用しています。.1は低音専用のサブウーファのことです。
これで十分満足していますが、スピーカマトリクスのサラウンドも体験してみたい。

# オペアンプでサラウンド回路を作る。
そこで、スピーカマトリクス法と同じR-L,L-R信号を生成するOPアンプの回路を作ってリアスピーカ用のパワーアンプ(PAM8403やST893)につないでみたところサラウンド感を知ることができました。
(回路図をsurroundAMP.png Kicad用データはsurroundAMP.kicad_schとして掲載）

聞いてみると確かにR-Lのような差分は、ステレオマイクの中央部以外からくる音を再生できています。
R-LとL-Rは逆相になるので音源の所在感はなく、2ch音源のライブを再生するとリアから拍手が広がって聞こえます。

これは面白いですね。

# MX-15 をESP32-A2DPで実現してみる。

有名な長岡鉄男氏の設計したマトリックススピーカでは　左に2L-R 中央にR＋L　右に2R-L 信号が出せるようにスピーカーをつないでいました。どんな音が得られるのでしょうか。

そこで、ESP32-A2DPライブラリを活用して、Bluetoothで受信しコールバック関数の中で抽出できる2chステレオPCMデータを加工して左用2L-R,右用2R-Lを作ってみました。
センタースピーカ用のR＋L信号は2kオームの抵抗2本と100kオームで作るアナログミキサで生成できます。
ミキサ回路がない場合は、小さいスピーカーなら左右を寄せてしまえば空間的にR+Lになります。（2L-R + 2R-L => L＋R になるからです。)

DACにはPCM5102Aを接続しており、配線はESP32-A2DPライブラリの標準的なGPIOの使い方に合わせてあります。

DACの出力からスピーカを鳴らすにはパワーアンプは必要です。
センタースピーカーはパワーアンプ入りの小さなスピーカを使いました。
結果としては2.1ch+センタースピーカでは2.1chではなかったサラウンド感はありました。

リアスピーカも併せて使うと、すでにOP-ampによるサラウンド効果が入っていますので、
<pre> 
Left channel (2L-R)-(2R-L) => 3*(L-R) 
Right channel (2R-L)-(2L-R) => 3*(R-L) 
</pre>
となってリアスピーカからはサラウンド効果のある差分が強化されて出てきます。
お好みでリア用のパワーアンプのボリュームを変えてください。
