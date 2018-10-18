#ifndef AES_H
#define AES_H

#include <inttypes.h>

namespace ns3 {

#ifndef BYTE
  #define BYTE uint8_t
#endif
#ifndef WORD
  #define WORD uint32_t
#endif

//key length related parameters
#define NB 4
#define NK 4
#define KEY_SIZE NK*NB
#define NR NB+NK+2

union Word
{
  BYTE b[4];
  WORD w;
};

class AES
{
public:
  /**
   * @brief AES constructor allocates memory for the state array
   */
  AES();

  /**
   * @brief AES destructor cleans up memory
   */
  ~AES();

  /**
   * @brief Encrypts the input data
   * @param input the pointer to the input information
   */
  void Encrypt(BYTE *input, int size);

  /**
   * @brief Decrypts the data, stored at output pointer
   * @param output the pointer to the input and output information
   */
  void Decrypt(BYTE *output, int size);

  /**
   * @brief SetKey sets the key for cipher
   * @param key the key
   * @param size length of the key
   */
  void SetKey(BYTE *key, int size);
private:
  /**
   * @brief state the main element of cipher - matrix 4xNB
   * used for store information during cipher process
   */
  Word state[NB];

  /**
   * @brief KeySchedule used to store round keys
   */
  Word keySchedule[NB * (NR + 1)];

  /**
   * @brief InputToState writes input into state
   * @param input the pointer to the input data
   */
  void InputToState(Word *input);

  /**
   * @brief StateToOutput saves state to the output
   * @param output the pointer for the output data
   */
  void StateToOutput(Word *output);

  //Here are functions that will be needed for encryption

  /**
   * @brief Cipher encrypts the data
   * @param data the pointer to the data
   */
  void Cipher(Word *data);

  /**
   * @brief ExpandKey expands key and creates round keys that will be stored in KeySchedule
   * @param key the pointer to the key data
   */
  void ExpandKey(Word *key);

  /**
   * @brief SubBytes makes byte substitution in the input
   * @param input word to which we apply subtitution
   * @param isInverse if true makes it run with InvSBox, otherwise with SBox
   */
  void SubBytes(Word &input, bool isInverse = false);

  /**
   * @brief ShiftRows shifts rows of the state as follows:
   * 1st row is unchanged
   * 2nd row shifts over one to the left
   * 3rd row shifts over two to the left
   * 4th row shifts over three to the left
   */
  void ShiftRows();

  /**
   * @brief MixColumns multiplies state matrix in Galua field
   * in column-by-column way each column multiplies with matrix
   * / {02} {03} {01} {01} \
   * | {01} {02} {03} {01} |
   * | {01} {01} {02} {03} |
   * \ {03} {01} {01} {02} /
   */
  void MixColumns();

  /**
   * @brief AddRoundKey adds round key to the state
   * @param round the current round number
   */
  void AddRoundKey(int round);

  /**
   * @brief RotWord rotates the word
   * @param data the word to rotate
   * @param isRightRot is it rotation to the right (left rotation used for encryption,
   * right rotation for decryption)
   * @param count amount of bytes over which will be rotated
   */
  void RotWord(Word &data, bool isRightRot = false, int count = 1);

  //Here are functions for decryption, they have prefix Inv in their names
  //some of needed functions was declared above, they have bool variables
  //to control their behaviour

  /**
   * @brief InvCipher decrypts the data
   * @param data the pointer to the data
   */
  void InvCipher(Word *data);

  /**
   * @brief InvShiftRows shifts rows of the state as follows:
   * 1st row is unchanged
   * 2nd row shifts over one to the right
   * 3rd row shifts over two to the right
   * 4th row shifts over three to the right
   */
  void InvShiftRows();

  /**
   * @brief InvMixColumns multiplies state matrix in Galua field
   */
  void InvMixColumns();

  //some magic for galua field
  //we need only multiplication by fixed constants
  //here will be functions for it
  BYTE MulBy02(BYTE multiplier);
  BYTE MulBy03(BYTE multiplier);
  BYTE MulBy09(BYTE multiplier);
  BYTE MulBy0B(BYTE multiplier);
  BYTE MulBy0D(BYTE multiplier);
  BYTE MulBy0E(BYTE multiplier);
};

} // namespace ns3

#endif // AES_H