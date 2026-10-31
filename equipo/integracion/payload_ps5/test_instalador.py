import unittest
from pathlib import Path
from instalar_ftp import updated, REMOTE
from generar_tabla import generate
class Tests(unittest.TestCase):
 def test_append_preserves_existing(self):
  old=b'idiomads.elf\r\n!3000\r\n'
  new=updated(old)
  self.assertTrue(new.startswith(old))
  self.assertIn(REMOTE.encode(),new)
  self.assertEqual(updated(new),new)
 def test_bare_name_not_duplicated(self):
  old=b'idiomads.elf\npes2021_fc.elf'
  self.assertEqual(updated(old),old)
 def test_missing_final_newline(self):
  self.assertEqual(updated(b'idiomads.elf'),b'idiomads.elf\n'+REMOTE.encode()+b'\n')
 def test_catalog_matches_generated(self):
  self.assertEqual(Path(__file__).with_name('tabla.h').read_text(encoding='utf-8-sig'),generate())
if __name__=='__main__': unittest.main()
