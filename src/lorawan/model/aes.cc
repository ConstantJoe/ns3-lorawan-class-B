#include "aes.h"

AES::AES()
{
  for (int i = 0; i < NB; i++)
    state[i].w = 0x00000000;
  for (int i = 0; i < NB * (NR + 1); i++)
    keySchedule[i].w = 0x00000000;
}

AES::~AES()
{
  for (int i = 0; i < NB; i++)
    state[i].w = 0x00000000;
  for (int i = 0; i < NB * (NR + 1); i++)
    keySchedule[i].w = 0x00000000;
}

void AES::Encrypt(BYTE *input, int size)
{
  int outSize = (size % 16 == 0? size: size + 16 - size % 16);
  BYTE *ptrPos = input;
  //padding
  if (outSize != size)
    {
      ptrPos += size;
      *ptrPos++ = 0x01;
      for (int i = 1; i < outSize - size; i++)
        *ptrPos++ = 0x00;
    }
  ptrPos = input;
  for (int i = 0; i < outSize; i+=16)
    {
      Cipher ((Word *)ptrPos);
      ptrPos+=16;
    }
}

void AES::Decrypt(BYTE *output, int size)
{
  BYTE *ptrPos = output;
  for (int i = 0; i < size; i+=16)
    {
      InvCipher ((Word *)ptrPos);
      ptrPos+=16;
    }
}

void AES::SetKey(BYTE *key, int size)
{
  if (size < KEY_SIZE)
    {
      BYTE *pos = key, *start = key;
      pos+=(size + 1);
      for (int i = size; i < KEY_SIZE; i++, pos++, start++)
        *pos = *start;
    }
  ExpandKey ((Word *)key);
}

void AES::InputToState(Word *input)
{
  Word *pos = input;
  for (int i = 0; i < NB; i++, pos++)
    state[i].w = pos->w;
  pos = 0;
}

void AES::StateToOutput(Word *output)
{
  Word *pos = output;
  for (int i = 0; i < NB; i++, pos++)
    {
      pos->w = state[i].w;
      state[i].w = 0x00000000;
    }
  pos = 0;
}

void AES::Cipher(Word *data)
{
  InputToState (data);

  AddRoundKey (0);

  for (int i = 1; i < NR; i++)
    {
      SubBytes (state[0], false);
      SubBytes (state[1], false);
      SubBytes (state[2], false);
      SubBytes (state[3], false);
      ShiftRows ();
      MixColumns ();
      AddRoundKey (i);
    }

  SubBytes (state[0], false);
  SubBytes (state[1], false);
  SubBytes (state[2], false);
  SubBytes (state[3], false);
  ShiftRows ();
  AddRoundKey (NR);

  StateToOutput (data);
}

void AES::ExpandKey(Word *key)
{
  Word *pos = key, temp;
  for (int i = 0; i < NK; i++)
      keySchedule[i].w = pos++->w;
  for (int i = NK; i < NB * (NR + 1); i++)
    {
      temp.w = keySchedule[i - 1].w;
      if (i % NK == 0)
        {
          RotWord (temp);
          //RotWord (temp, false, 1);
          SubBytes (temp, false);
          temp.b[0] ^= Rcon[i / NK];
        }
      keySchedule[i].w = keySchedule[i - NK].w ^ temp.w;
    }
}

void AES::SubBytes(Word &input, bool isInverse)
{
  if (!isInverse)
    for (int j = 0; j < 4; j++)
      input.b[j] = SBox[input.b[j]];
  else
    for (int j = 0; j < 4; j++)
      input.b[j] = InvSBox[input.b[j]];
}

void AES::ShiftRows()
{
  BYTE temp = 0x00;
  //first row
  temp = state[0].b[1];
  state[0].b[1] = state[1].b[1];
  state[1].b[1] = state[2].b[1];
  state[2].b[1] = state[3].b[1];
  state[3].b[1] = temp;
  //second row
  temp = state[0].b[2];
  state[0].b[2] = state[1].b[2];
  state[1].b[2] = state[2].b[2];
  state[2].b[2] = state[3].b[2];
  state[3].b[2] = temp;
  temp = state[0].b[2];
  state[0].b[2] = state[1].b[2];
  state[1].b[2] = state[2].b[2];
  state[2].b[2] = state[3].b[2];
  state[3].b[2] = temp;
  //third row
  temp = state[3].b[3];
  state[3].b[3] = state[2].b[3];
  state[2].b[3] = state[1].b[3];
  state[1].b[3] = state[0].b[3];
  state[0].b[3] = temp;

  temp = 0x00;
}

void AES::MixColumns()
{
  Word temp;
  for (int i = 0; i < NB; i++)
    {
      temp.w = state[i].w;
      state[i].b[0] = MulBy02 (temp.b[0]) ^ MulBy03 (temp.b[1]) ^ temp.b[2] ^ temp.b[3];
      state[i].b[1] = temp.b[0] ^ MulBy02 (temp.b[1]) ^ MulBy03 (temp.b[2]) ^ temp.b[3];
      state[i].b[2] = temp.b[0] ^ temp.b[1] ^ MulBy02 (temp.b[2]) ^ MulBy03 (temp.b[3]);
      state[i].b[3] = MulBy03 (temp.b[0]) ^ temp.b[1] ^ temp.b[2] ^ MulBy02 (temp.b[3]);
    }
  temp.w = 0x00000000;
}

void AES::AddRoundKey(int round)
{
  for (int i = 0; i < NB; i++)
    state[i].w ^= keySchedule[round*NB + i].w;
}

void AES::RotWord(Word &data, bool isRightRot, int count)
{
  BYTE temp = 0x00;
  if (!isRightRot)
    for (int i = 0; i < count; i++)
      {
        temp = data.b[0];
        data.b[0] = data.b[1];
        data.b[1] = data.b[2];
        data.b[2] = data.b[3];
        data.b[3] = temp;
      }
  else
    for (int i = 0; i < count; i++)
      {
        temp = data.b[3];
        data.b[3] = data.b[2];
        data.b[2] = data.b[1];
        data.b[1] = data.b[0];
        data.b[0] = temp;
      }
  temp = 0x00;
}

void AES::InvCipher(Word *data)
{
  InputToState (data);

  AddRoundKey (NR);

  for (int i = NR - 1; i >= 1; i--)
    {
      InvShiftRows ();
      SubBytes (state[0], true);
      SubBytes (state[1], true);
      SubBytes (state[2], true);
      SubBytes (state[3], true);
      AddRoundKey (i);
      InvMixColumns ();
    }

  InvShiftRows ();
  SubBytes (state[0], true);
  SubBytes (state[1], true);
  SubBytes (state[2], true);
  SubBytes (state[3], true);
  AddRoundKey (0);

  StateToOutput (data);
}

void AES::InvShiftRows()
{
  BYTE temp = 0x00;
  //third row
  temp = state[0].b[3];
  state[0].b[3] = state[1].b[3];
  state[1].b[3] = state[2].b[3];
  state[2].b[3] = state[3].b[3];
  state[3].b[3] = temp;
  //second row
  temp = state[0].b[2];
  state[0].b[2] = state[1].b[2];
  state[1].b[2] = state[2].b[2];
  state[2].b[2] = state[3].b[2];
  state[3].b[2] = temp;
  temp = state[0].b[2];
  state[0].b[2] = state[1].b[2];
  state[1].b[2] = state[2].b[2];
  state[2].b[2] = state[3].b[2];
  state[3].b[2] = temp;
  //first row
  temp = state[3].b[1];
  state[3].b[1] = state[2].b[1];
  state[2].b[1] = state[1].b[1];
  state[1].b[1] = state[0].b[1];
  state[0].b[1] = temp;

  temp = 0x00;
}

void AES::InvMixColumns()
{
  Word temp;
  for (int i = 0; i < NB; i++)
    {
      temp.w = state[i].w;
      state[i].b[0] = MulBy0E (temp.b[0]) ^ MulBy0B (temp.b[1]) ^ MulBy0D (temp.b[2]) ^ MulBy09 (temp.b[3]);
      state[i].b[1] = MulBy09 (temp.b[0]) ^ MulBy0E (temp.b[1]) ^ MulBy0B (temp.b[2]) ^ MulBy0D (temp.b[3]);
      state[i].b[2] = MulBy0D (temp.b[0]) ^ MulBy09 (temp.b[1]) ^ MulBy0E (temp.b[2]) ^ MulBy0B (temp.b[3]);
      state[i].b[3] = MulBy0B (temp.b[0]) ^ MulBy0D (temp.b[1]) ^ MulBy09 (temp.b[2]) ^ MulBy0E (temp.b[3]);
    }
  temp.w = 0x00000000;
}

BYTE AES::MulBy02(BYTE multiplier)
{
  if (multiplier < 0x80)
    return (multiplier << 1);
  else
    return ((multiplier << 1) ^ 0x1b);
}

BYTE AES::MulBy03(BYTE multiplier)
{
  return MulBy02 (multiplier) ^ multiplier;
}

BYTE AES::MulBy09(BYTE multiplier)
{
  return MulBy02 (MulBy02 (MulBy02 (multiplier))) ^ multiplier;
}

BYTE AES::MulBy0B(BYTE multiplier)
{
  return MulBy02 (MulBy02 (MulBy02 (multiplier))) ^ MulBy02 (multiplier) ^ multiplier;
}

BYTE AES::MulBy0D(BYTE multiplier)
{
  return MulBy02 (MulBy02 (MulBy02 (multiplier))) ^ MulBy02 (MulBy02 (multiplier)) ^ multiplier;
}

BYTE AES::MulBy0E(BYTE multiplier)
{
  return MulBy02 (MulBy02 (MulBy02 (multiplier))) ^ MulBy02 (MulBy02 (multiplier)) ^ MulBy02 (multiplier);
}