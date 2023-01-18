# RCOM-2
## Formato e tipos de tramas

* A transmissão é organizada em tramas, que podem ser de três tipos:

    > Informação (I)<br>
    > Supervisão (S) <br>
    > Não Numeradas (U)

* Apenas as tramas de Informação possuem um campo para transporte de dados

* As tramas são protegidas por um código detetor de erros
    * Nas tramas S e U existe proteção simples da trama
    * Nas tramas I existe proteção dupla e independente do cabeçalho e do campo de dados

<br>

![frame](https://user-images.githubusercontent.com/92220581/198890864-c468b523-b896-4d51-ac8e-156a8871139d.jpg)

## Tramas - Delimitação de cabeçalho

* Todas as tramas são delimitadas por **flags** (Ob01111110, Ox7e) e pode ser iniciada por mais do que uma, o que deve ser
tido em conta pelo mecanismo de recepção de tramas

* Tramas I, SET e DISC são designadas **Comandos** e as tramas UA,
RR e REJ **Respostas**

* As tramas têm um cabeçalho com um formato comum
    * **A (Campo de Endereço)**
        * 00000011 (0x03) em Comandos enviados pelo Emissor e Respostas
enviadas pelo Receptor
        * 00000001 (0x01) em Comandos enviados pelo Receptor e Respostas
enviadas pelo Emissor 

    * **C (Campo de Controlo)**
        * Define o tipo de trama e transporta números de sequência N(s) em tramas I e N(r) em tramas de Supervisão (RR, REJ)

    * **BCC (Block Check Character)** 
        * Detecção de erros baseada na geração de um octeto (BCC) tal que exista um número par de 1s em cada posição (bit), considerando todos os octetos protegidos pelo BCC (cabeçalho ou dados, conforme os casos) e o próprio BCC (antes de stuffing)

### Receção de tramas - Procedimento

* Tramas I, S ou U com cabeçalho errado são ignoradas, sem qualquer ação

* O campo de dados das tramas I é protegido por um BCC próprio (paridade
par sobre cada um dos bits dos octetos de dados e do BCC)

* Tramas I recebidas **sem erros detetados** no cabeçalho e no campo de dados
são aceites para processamento

    * Se se tratar duma nova trama, o campo de dados é aceite (e passado à Aplicação), e a trama deve ser confirmada com RR

    * Se se tratar dum duplicado, o campo de dados é descartado, mas deve fazer-se confirmação da trama com RR

* Tramas I sem erro detetado no cabeçalho mas **com erro detectado no campo de dados** (pelo respectivo BCC) – o campo de dados é descartado, mas o
campo de controlo pode ser usado para desencadear uma ação adequada

    * Se se tratar duma nova trama, é conveniente fazer um pedido de retransmissão com REJ, o que permite antecipar a ocorrência de time-out no emissor

    * Se se tratar dum duplicado, deve fazer-se confirmação com RR

* Tramas I, SET e DISC são protegidas por um temporizador
    * Em caso de ocorrência de time-out, deve ser efetuado um número máximo de tentativas de retransmissão

## Transparência

> Garante transmissão de dados independente de códigos. É assegurada pela técnica de *byte stuffing*

### Necessidade

* Transmissão assíncrona

    * Caracteriza-se pela transmissão de “caracteres” (sequência curta de bits, cujo número pode ser configurado) delimitados por bits Start e Stop
    
* O protocolo a implementar não se baseia na utilização de qualquer código,
pelo que os caracteres transmitidos (constituídos por 8 bits) devem ser
interpretados como simples octetos (bytes), podendo ocorrer qualquer uma
das 256 combinações possíveis

* Para evitar o falso reconhecimento de uma flag no interior de uma trama, é
necessário um mecanismo que garanta transparência

### Mecanismo de *byte stuffing*

> Evita o falso reconhecimento de uma flag no interior de uma trama

* No protocolo a implementar adota-se o mecanismo usado em PPP, que
recorre ao octeto de escape 01111101 (0x7d)

    * Se no interior da trama ocorrer o octeto 01111110 (0x7e), isto é, o padrão que corresponde a uma flag, o octeto é substituído pela sequência 0x7d 0x5e (octeto de escape seguido do resultado do ou exclusivo do octeto substituído com o octeto 0x20)

    * Se no interior da trama ocorrer o octeto 01111101 (0x7d), isto é, o padrão que corresponde ao octeto de escape, o octeto é substituído pela sequência 0x7d 0x5d (octeto de escape seguido do resultado do ou exclusivo do octeto substituído com o octeto 0x20)

    * Na geração do BCC são considerados apenas os octetos originais (antes da operação de stuffing), mesmo que algum octeto (incluindo o próprio BCC) tenha de ser substituído pela correspondente sequência de escape
    
    * A verificação do BCC é feita em relação aos octetos originais, isto é, depois de realizada a operação inversa (destuffing), caso tenha ocorrido na emissão a substituição de algum dos octetos especiais pela correspondente sequência de escape

## Transferência de dados – Retransmissões
* Confirmação / Controlo de Erros
    * Stop-and-Wait
* Temporizador
    * Activado após o envio de uma trama I, SET ou DISC
    * Desactivado após recepção de uma resposta válida
    * Se excedido (time-out), obriga a retransmissão
* Retransmissão de tramas I
    * Se ocorrer time-out, devido à perda da trama I enviada ou da sua confirmação
        * Número máximo predefinido (configurado) de tentativas de retransmissão
    * Após receção de confirmação negativa (REJ)
* Protecção da trama
    * Geração e verificação do(s) campo(s) de proteção (BCC)

## Pacotes e tramas

* O ficheiro a transmitir é fragmentado – os fragmentos são
encapsulados em pacotes de dados e estes são transportados no
campo de dados de tramas I

* Designa-se por **Emissor** a máquina que envia o ficheiro e por
**Receptor** a máquina que recebe o ficheiro
    * Apenas o Emissor transmite pacotes (de dados ou de controlo) e
portanto apenas o Emissor transmite tramas I

### Aplicação de teste – Especificação

* A aplicação deve suportar dois tipos de pacotes enviados pelo Emissor

    > **Pacotes de controlo** para sinalizar o início e o fim da transferência do ficheiro <br>
    > **Pacotes de dados** contendo fragmentos do ficheiro a transmitir

* O **pacote de controlo que sinaliza o início da transmissão** (start) deverá ter obrigatoriamente um campo com o tamanho do ficheiro e opcionalmente um campo com o nome do ficheiro (e eventualmente outros campos)

* O **pacote de controlo que sinaliza o fim da transmissão** (end) deverá repetir a informação contida no pacote de início de transmissão

* Os **pacotes de dados** contêm obrigatoriamente um campo (um octeto) com um número de sequência e um campo (dois octetos) que indica o tamanho do respectivo campo de dados

    * Este tamanho depende do tamanho máximo do campo de Informação das tramas I

    * Estes campos permitem verificações adicionais em relação à integridade dos
dados

    * A numeração de pacotes de dados é independente da numeração das tramas I

![packet](https://user-images.githubusercontent.com/92220581/198890881-9a0667db-c25a-45cb-9082-6180618682b4.jpg)
